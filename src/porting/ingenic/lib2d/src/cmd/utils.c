int read_raw(FILE *fp, void *src, int height, int src_stride, int dst_stride)
{
    size_t read_size = 0;
    int i = 0;

    for (i = 0; i < height; i++) {
        read_size = fread(src, sizeof(char), src_stride, fp);
        if(read_size != src_stride) {
            fprintf(stderr, "mscaler: fread y fail, read_size(%d) != default_stride(%d)\n", read_size, src_stride);
            return -1;
        }

        src += dst_stride;
    }

    return 0;

}

int file_write_data(const char *out_file, void *data, unsigned long data_size)
{
    int ret = 0;
    FILE *file = fopen(out_file, "w");
    if (!file) {
        fprintf(stderr, "file_utils: failed to open file: %s (%s)\n",
            out_file, strerror(errno));
        return -1;
    }

    int N = 1024*1024;

    while (data_size) {
        int size = data_size < N ? data_size : N;
        ret = fwrite(data, 1, size, file);
        if (ret < 0) {
            fprintf(stderr, "file_utils: failed to write file: %s (%s) %d\n",
                out_file, strerror(errno), size);
            goto close_file;
        }

        data_size -= ret;
        data += ret;
    }

    ret = 0;
    fflush(file);

close_file:
    fclose(file);

    return ret;
}

void fb_display(int fb_fd, struct fb_device_info *info, struct ingenic_2d_frame *frame)
{
    struct lcdc_layer layer_cfg = {
        .xres = DST_WIDTH,
        .yres = DST_HEIGHT,
        .xpos = 0,
        .ypos = 0,
        .fb_fmt = fb_fmt_ARGB8888,

        .layer_order = lcdc_layer_0,
        .layer_enable = 1,

        .rgb = {
            .mem = (void *)frame->phyaddr[0],
            .stride = frame->stride,
        },

        .alpha = {
            .enable = 0,
        },

        .scaling = {
            .enable = 1,
            .xres = info->xres,
            .yres = info->yres,
        },
    };


    fb_pan_display_enable_user_cfg(fb_fd);
    fb_pan_display_set_user_cfg(fb_fd, &layer_cfg);

    fb_pan_display(fb_fd, info, 0);
}
