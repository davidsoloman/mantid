from __future__ import (absolute_import, division, print_function)


def _import_scipy(config):
    """
    Tries to import scipy so that the median filter can be applied
    :return:
    """

    scipy_ndimage = None
    try:
        import scipy.ndimage as scipy_ndimage
    except ImportError:
        if config.crash_on_failed_import:
            raise ImportError(
                "Could not find the subpackage scipy.ndimage, required for image pre-/post-processing"
            )
        else:
            from recon.helper import Helper
            h = Helper(config)

            h.tomo_print(
                "Could not find the package scipy, "
                "crash_on_failed_import is False, so continuing execution...", 1)

    return scipy_ndimage


def execute(data, config, median_filter_mode='mirror'):
    """

    :param data: The data that will be processed with this filter
    :param config: The reconstruction config
    :param median_filter_mode: Default: 'mirror', {'reflect', 'constant', 'nearest', 'mirror', 'wrap'}, optional
                The mode parameter determines how the array borders are handled, where cval is the value when
                mode is equal to 'constant'.
    :return: Returns the processed data
    """
    from recon.helper import Helper
    h = Helper(config)
    median_filter_size = config.pre.median_filter_size

    if median_filter_size and median_filter_size > 1:
        scipy_ndimage = _import_scipy(config)
        h.pstart(
            " * Starting noise filter / median, with pixel data type: {0}, filter size/width: {1}.".
            format(data.dtype, median_filter_size))

        for idx in range(0, data.shape[0]):
            data[idx] = scipy_ndimage.median_filter(
                data[idx], median_filter_size, mode=median_filter_mode)

        h.pstop(
            " * Finished noise filter / median, with pixel data type: {0}, filter size/width: {1}.".
            format(data.dtype, median_filter_size))

    else:
        h.tomo_print(" * Note: NOT applying noise filter /median.", 2)

    return data


def execute_ndimensional(data, config):
    raise NotImplementedError("Median filter 3D is disabled.")

    # noinspection PyUnreachableCode
    from recon.helper import Helper
    h = Helper(config)

    scipy_ndimage = _import_scipy(config)

    data = scipy_ndimage.median_filter(data,
                                       config.median_filter_size)

    return data


def execute_3d(data, config):
    raise NotImplementedError("Median filter 3D is disabled.")

    # noinspection PyUnreachableCode
    from recon.helper import Helper
    h = Helper(config)

    if config.median_filter3d_size and config.median_filter3d_size > 1:

        scipy_ndimage = _import_scipy(config)

        kernel_size = config.median_filter3d_size

        # # Note this can be extremely slow
        # data = scipy.signal.medfilt(
        #     data, kernel_size=kernel_size)
    else:
        h.tomo_print(
            " * Note: NOT applied N-dimensional median filter on reconstructed volume", 2)

    return data