from __future__ import (absolute_import, division, print_function)


import numpy as np


# TODO write performance test, execution and memory usage!

def read_in_stack(config):
    """
    Loads a stack, including sample, white and dark images.

    :config: The full reconstruction config

    :returns :: stack of images as a 3-elements tuple: numpy array with sample images, white image, and dark image.
    """

    img_format = config.pre.in_img_format
    sample_path = config.func.input_dir
    flat_field_path = config.func.input_dir_flat
    dark_field_path = config.func.input_dir_dark

    supported_exts = ['tiff', 'tif', 'fits', 'fit', 'png']

    if img_format not in supported_exts:
        raise ValueError("File extension not supported: {0}. Supported extensions: {1}".
                         format(img_format, supported_exts))

    sample, flat, dark = read_stack_of_images(
        sample_path, flat_field_path, dark_field_path, img_format)

    from recon.helper import Helper
    Helper.check_data_stack(sample)

    return sample, flat, dark


def read_stack_of_images(sample_path, flat_file_path=None, dark_file_path=None,
                         file_extension='tiff', file_prefix='',
                         flat_file_prefix='', dark_file_prefix=''):
    """
    Reads a stack of images into memory, assuming dark and flat images
    are in separate directories.

    If several files are found in the same directory (for example you
    give image0001.fits and there's also image0002.fits,
    image0003.fits) these will also be loaded as the usual convention
    in ImageJ and related imaging tools, using the last digits to sort
    the images in the stack.

    :param sample_path :: path to sample images. Can be a file or directory
    :param flat_file_path :: (optional) path to open beam / flat image(s). Can be a file or directory
    :param dark_file_path :: (optional) path to dark field image(s). Can be a file or directory
    :param file_extension :: file extension (typically 'tiff', 'tif', 'fits', or 'fit' (not including the dot)
    :param file_prefix :: prefix for the image files, to filter files that should not be loaded
    :param flat_file_prefix :: prefix for the flat field image files
    :param dark_file_prefix :: prefix for the dark field image files

    :return :: 3 numpy arrays: input data volume (3d), average of flatt images (2d),
    average of dark images(2d)
    """

    sample_file_names = _get_stack_file_names(
        sample_path, file_prefix, file_extension)

    flat_file_names = _get_stack_file_names(
        flat_file_path, flat_file_prefix, file_extension)

    dark_file_names = _get_stack_file_names(
        dark_file_path, dark_file_prefix, file_extension)

    # It is assumed that all images have the same size and properties as the
    # first.
    try:
        first_sample_img = _read_img(sample_file_names[0], file_extension)
    except RuntimeError as exc:
        raise RuntimeError(
            "Could not load at least one image file from: {0}. Details: {1}".
            format(sample_path, str(exc)))

    data_dtype = first_sample_img.dtype
    # usual type in fits with 16-bit pixel depth
    if '>i2' == data_dtype:
        # TODO test all data types:
        # uint16, float16, float32, float64!
        # Was getting bad data with float16 after median filter and scaling up
        # to save as PNG
        data_dtype = np.float16

    img_shape = first_sample_img.shape

    sample_data = _read_listed_files(
        sample_file_names, img_shape, file_extension, data_dtype)

    flat_data = _read_listed_files(
        flat_file_names, img_shape, file_extension, data_dtype)
    flat_avg = get_data_average(flat_data)

    dark_data = _read_listed_files(
        dark_file_names, img_shape, file_extension, data_dtype)
    dark_avg = get_data_average(dark_data)

    return sample_data, flat_avg, dark_avg


def _get_stack_file_names(path, file_prefix, file_extension):
    import os
    import glob
    from recon.helper import Helper
    h = Helper()

    path = os.path.expanduser(path)

    h.tomo_print("Loading stack of images from {0}".format(path))

    files_match = glob.glob(os.path.join(
        path, "{0}*.{1}".format(file_prefix, file_extension)))

    if len(files_match) <= 0:
        raise RuntimeError("Could not find any image files in {0}, with prefix: {1}, extension: {2}".
                           format(path, file_prefix, file_extension))
    # files_match.sort(key=_alphanum_key_split)

    h.tomo_print("Found {0} image files in {1}".format(
        len(files_match), path))

    return files_match


def _alphanum_key_split(path_str):
    """
    From a string to a list of alphabetic and numeric elements. Intended to
    be used for sequence number/natural sorting. In list.sort() the
    key can be a list, so here we split the alpha/numeric fields into
    a list. For example (in the final order after sort() would be applied):

    "angle4" -> ["angle", 4]
    "angle31" -> ["angle", 31]
    "angle42" -> ["angle", 42]
    "angle101" -> ["angle", 101]

    Several variants compared here:
    https://dave.st.germa.in/blog/2007/12/11/exception-handling-slow/
    """
    import re
    alpha_num_split_re = re.compile('([0-9]+)')
    return [int(c) if c.isdigit() else c for c in alpha_num_split_re.split(path_str)]


def _read_img(filename, file_extension=None):
    """
    Read one image and return it as a 2d numpy array

    :param filename :: name of the image file, can be relative or absolute path
    :param file_extension :: extension and effectively format to use ('tiff', 'fits')
    """
    if file_extension in ['fits', 'fit']:
        pyfits = import_pyfits()
        image = pyfits.open(filename)
        if len(image) < 1:
            raise RuntimeError(
                "Could not load at least one FITS image/table file from: {0}".format(filename))

        # Input fits files always contain a single image
        img_arr = image[0].data

    elif file_extension in ['tiff', 'tif', 'png']:
        skio = import_skimage_io()
        img_arr = skio.imread(filename)

    else:
        raise ValueError(
            "Don't know how to load a file with this extension: {0}".format(file_extension))

    return img_arr


def import_pyfits():
    """
    To import pyfits optionally only when it is/can be used
    """
    try:
        import pyfits
    except ImportError:
        # In Anaconda python, the pyfits package is in a different place, and this is what you frequently
        # find on windows.
        try:
            import astropy.io.fits as pyfits
        except ImportError:
            raise ImportError(
                "Cannot find the package 'pyfits' which is required to read/write FITS image files")

    return pyfits


def import_skimage_io():
    """
    To import skimage io only when it is/can be used
    """
    try:
        from skimage import io as skio
        skio.use_plugin('freeimage')
    except ImportError as exc:
        raise ImportError("Could not find the package skimage, its subpackage "
                          "io and the pluging freeimage which are required to support "
                          "several image formats. Error details: {0}".format(exc))
    return skio


def _read_listed_files(files, slice_img_shape, file_extension, dtype):
    """
    Read several images in a row into a 3d numpy array. Useful when reading all the sample
    images, or all the flat or dark images.

    :param files :: list of image file paths given as strings
    :param slice_img_shape :: shape of every image
    :param file_extension :: file name extension if fixed (to set the expected image format)
    :param dtype :: data type for the output numpy array

    Returns:: a 3d data volume with the size of the first (outermost) dimension equal
    to the number of files, and the sizes of the second and third dimensions equal to
    the sizes given in the input slice_img_shape
    """

    # Zeroing here is good, because we are going to load the data in anyway.
    # FIXME TODO PERFORMANCE CRITICAL
    # TODO Performance Tests

    data = np.zeros((len(files), slice_img_shape[
                    0], slice_img_shape[1]), dtype=dtype)

    for idx, in_file in enumerate(files):
        try:
            data[idx, :, :] = _read_img(in_file, file_extension)
        except IOError as exc:
            raise RuntimeError(
                "Could not load file {0}. Error details: {1}".format(in_file, str(exc)))

    return data


def get_data_average(data):
    avg = np.mean(data, axis=0)
    return avg