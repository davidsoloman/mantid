from __future__ import (absolute_import, division, print_function)


def save_single_image(data,
                      config=None,
                      output_path=None,
                      image_name='saved_image',
                      image_index=0):
    """
    Save (pre-processed) images from a data array to image files.
    :param config:
    :param output_path :: additional output directory, currently used for debugging
    :param data :: data volume with pre-processed images
    :param image_name: image name to be appended
    :param image_index: image index to be appended
    """
    from recon.helper import Helper
    import os

    # using the config's output dir
    preproc_dir = os.path.join(
        config.func.output_path, config.func.preproc_subdir)

    h = Helper(config)

    if output_path is not None:
        # using the provided output_path
        preproc_dir = os.path.join(preproc_dir, output_path)

    h.pstart(
        " * Saving single image {0} dtype: {1}".format(preproc_dir, data.dtype))

    make_dirs_if_needed(preproc_dir)

    write_image(data, os.path.join(
        preproc_dir, image_name + str(image_index).zfill(6)))

    h.pstop(" * Finished saving single image.")


def save_recon_output(data, config):
    """
    Save output reconstructed volume in different forms.

    :param data :: reconstructed data volume. A sequence of images will be saved from this
    :param config :: configuration of the reconstruction, including output paths and formats
    slices saved by default. Useful for testing some tools
    """
    # slices along the vertical (z) axis
    # output_path = 'output_recon_tomopy'
    from recon.helper import Helper
    h = Helper(config)

    import os

    output_path = config.func.output_path
    out_recon_dir = os.path.join(output_path, 'reconstructed')
    data_as_stack = config.func.data_as_stack
    h.pstart(
        " * Starting saving slices of the reconstructed volume in: {0}...".format(out_recon_dir))

    save_recon_as_vertical_slices(
        data, out_recon_dir, config.func.out_slices_file_name_prefix, data_as_stack)

    # Sideways slices:
    # if config.func.save_horiz_slices:
    out_horiz_dir = os.path.join(output_path, 'horiz_slices')
    print("* Saving horizontal slices in: {0}".format(out_horiz_dir))
    save_recon_as_horizontal_slices(
        data, out_horiz_dir, config.func.out_horiz_slices_subdir, data_as_stack)

    h.pstop(
        " * Finished saving slices of the reconstructed volume in: {0}".
        format(out_recon_dir))


def save_recon_as_vertical_slices(data, output_dir, name_prefix='out_recon_slice', data_as_stack=False):
    """
    Save reconstructed volume (3d) into a series of slices along the Z axis (outermost numpy dimension)
    :param data_as_stack:
    :param data :: data as images/slices stores in numpy array
    :param output_dir :: where to save the files
    :param name_prefix :: prefix for the names of the images - an index is appended to this prefix
    """

    import os

    make_dirs_if_needed(output_dir)
    if not data_as_stack:
        for idx in range(0, data.shape[0]):
            write_image(data[idx, :, :], os.path.join(
                output_dir, name_prefix + str(idx).zfill(6)))
    else:
        write_image(data, os.path.join(
            output_dir, name_prefix + "_stack".zfill(6)))


def save_recon_as_horizontal_slices(data, out_horiz_dir, name_prefix='out_recon_horiz_slice', data_as_stack=False):
    """
    Save reconstructed volume (3d) into a series of slices along the Y axis (second numpy dimension)
    :param data :: data as images/slices stores in numpy array
    :param out_horiz_dir :: where to save the files
    :param name_prefix :: prefix for the names of the images throughout the horizontal axis
    the prefix
    """
    import os
    make_dirs_if_needed(out_horiz_dir)

    # TODO find out how to flip with numpy
    # if not config.func.data_as_stack:
    for idx in range(0, data.shape[1]):
        write_image(data[:, idx, :], os.path.join(
            out_horiz_dir, name_prefix + str(idx).zfill(6)))
    # else:
        # write_image(data[:, idx, :], os.path.join(
        # out_horiz_dir, name_prefix + str(idx).zfill(6)))


def save_preproc_images(data, config):
    """
    Save (pre-processed) images from a data array to image files.

    :param data :: The pre-processed data that will be saved
    :param config :: The full reconstruction config
    """
    from recon.helper import Helper
    h = Helper(config)

    if not config.func.save_preproc:
        h.tomo_print_warning(
            "NOT saving out pre-processed images, because no -s/--save-preproc was specified.")
        return

    import os

    preproc_dir = os.path.join(
        config.func.output_path, config.func.preproc_subdir)

    h.pstart(
        " * Saving all pre-processed images (data) into {0} dtype: {1}".format(preproc_dir, data.dtype))

    make_dirs_if_needed(preproc_dir)

    if not config.func.data_as_stack:
        for idx in range(0, data.shape[0]):
            write_image(data[idx, :, :], os.path.join(
                preproc_dir, 'out_preproc_proj_image' + str(idx).zfill(6)))
    else:
        write_image(data, os.path.join(
            preproc_dir, 'out_preproc_proj_images_stack'))

    h.pstop(" * Saving pre-processed images finished.")


def write_image(img_data, filename):
    """
    Output image data, given as a numpy array, to a file, in a given image format.
    Assumes that the output directory exists (must be checked before). The pixel
    values are rescaled in the range [min_pix, max_pix] which would normally be set
    to the minimum/maximum values found in a stack of images.

    :param img_data :: image data in the usual numpy representation
    :param filename :: file name, including directory and extension
    of the input data is used

    :returns:: name of the file saved
    """

    from recon.data import loader

    # from skimage import exposure
    # img_data = exposure.rescale_intensity(img_data[:, :], out_range='uint16')

    fits = loader.import_pyfits()
    # to flip vertically img_data[::-1] ps. saved images are still flipped
    # vertically
    hdu = fits.PrimaryHDU(img_data)
    hdulist = fits.HDUList([hdu])
    hdulist.writeto(filename + ".fits")

    return filename


def gen_readme_summary_begin(filename, config, cmd_line):
    """
    To write configuration, settings, etc. early on. As early as possible, before any failure
    can happen.

    @param filename :: name of the readme/final report file
    @param config :: full reconstruction configuration
    @param cmd_line :: command line originally used to run this reconstruction, when running
    from the command line

    Returns :: time now (begin of run) in number of seconds since epoch (time() time)
    """

    import time
    tstart = time.time()

    make_dirs_if_needed(config.func.output_path)

    # generate file with dos/windows line end for windows users convenience
    with open(filename, 'w') as oreadme:
        file_hdr = (
            'Tomographic reconstruction. Summary of inputs, settings and outputs.\n'
            'Time now (run begin): ' + time.ctime(tstart) + '\n')
        oreadme.write(file_hdr)

        alg_hdr = ("\n"
                   "--------------------------\n"
                   "Tool/Algorithm\n"
                   "--------------------------\n")
        oreadme.write(alg_hdr)
        oreadme.write(str(config.func))
        oreadme.write("\n")

        preproc_hdr = ("\n"
                       "--------------------------\n"
                       "Pre-processing parameters\n"
                       "--------------------------\n")
        oreadme.write(preproc_hdr)
        oreadme.write(str(config.pre))
        oreadme.write("\n")

        postproc_hdr = ("\n"
                        "--------------------------\n"
                        "Post-processing parameters\n"
                        "--------------------------\n")
        oreadme.write(postproc_hdr)
        oreadme.write(str(config.post))
        oreadme.write("\n")

        cmd_hdr = ("\n"
                   "--------------------------\n"
                   "Command line\n"
                   "--------------------------\n")
        oreadme.write(cmd_hdr)
        oreadme.write(cmd_line)
        oreadme.write("\n")


def make_dirs_if_needed(dirname):
    """
    Makes sure that the directory needed (for example to save a file)
    exists, otherwise creates it.

    @param dirname :: (output) directory to check

    """
    import os
    absname = os.path.abspath(dirname)
    if not os.path.exists(absname):
        os.makedirs(absname)


def gen_readme_summary_end(filename, data_stages, tstart,
                           t_recon_elapsed):
    """
    Write last part of report in the output readme/report file. This should be used whenever a
    reconstruction runs correctly.

    @param filename :: name of the readme/final report file
    @param data_stages :: tuple with data in three stages (raw, pre-processed, reconstructed)
    @param tstart :: time at the beginning of the job/reconstruction, when the first part of the
    readme file was written
    @param t_recon_elapsed :: reconstruction time
    """
    # append to a readme/report that should have been pre-filled with the
    # initial configuration
    with open(filename, 'a') as oreadme:
        import time
        run_hdr = ("\n"
                   "--------------------------\n"
                   "Run/job details:\n"
                   "--------------------------\n")
        oreadme.write(run_hdr)
        (raw_data, preproc_data, data) = data_stages

        oreadme.write("Dimensions of raw input sample data: {0}\n".format(
            raw_data.shape))
        oreadme.write("Dimensions of pre-processed sample data: {0}\n".
                      format(preproc_data.shape))
        oreadme.write("Dimensions of reconstructed volume: {0}\n".format(
            data.shape))

        oreadme.write("Raw input pixel type: {0}\n".format(raw_data.dtype))
        oreadme.write("Output pixel type: {0}\n".format('uint16'))
        oreadme.write("Time elapsed in reconstruction: {0:.3f}s\r\n".
                      format(t_recon_elapsed))
        tend = time.time()
        oreadme.write('Time now (run end): ' + time.ctime(tend))
