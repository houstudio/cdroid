#ifndef __LIBHARDWARE2_NEMC_H__
#define __LIBHARDWARE2_NEMC_H__

#ifdef  __cplusplus
extern "C" {
#endif

struct nemc_timing {
    uint32_t tas;               /* Tas */
    uint32_t taw;               /* Taw */
    uint32_t tbp;               /* Tbp */
    uint32_t tah;               /* Tah */
    uint32_t strv;              /* strv */
};

int nemc_open(const char *dev_path, void **mem);
int nemc_set_timing(int fd, struct nemc_timing *timing);
int nemc_get_timeing(int fd, struct nemc_timing *timing);
void nemc_close(int fd, void *mem);



#ifdef  __cplusplus
}
#endif

#endif

