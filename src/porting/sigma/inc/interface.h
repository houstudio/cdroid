#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Open file or url and set the windows size
 */
int mm_player_open(const char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * Close file or url
 */
int mm_player_close(void);

/**
 * Stop playing
 */
int mm_player_pause(void);

/**
 * Resume playing
 */
int mm_player_resume(void);

/**
 * Seek file forward or backward in the current position for time
 */
int mm_player_seek(double time);

/**
 * Seek file to the setting time
 */
int mm_player_seek2time(double time);

/**
 * Get the current playing time of video file
 */
int mm_player_getposition(double *position);

/**
 * Get the total time of video file
 */
int mm_player_getduration(double *duration);

/**
 * Set the audio volumn.
 * volumn = [0]~[100]
 */
int mm_player_set_volumn(int volumn);

/**
 * Mute the audio.
 * volumn = false/true
 */
int mm_player_set_mute(bool mute);

/**
 * Set the windows size.
 * The function is valid when we call mm_player_set_opts("enable_scaler", "", 1) before mm_player_open.
 */
int mm_player_set_window(int x, int y, int width, int height);

/**
 * Set player others options.
 * key: option name; value: reserved; flags: option value. Such as:
 * mm_player_set_opts("video_only", "", 0); -- "1"=enable; "0"=disable
 * mm_player_set_opts("audio_only", "", 0); -- "1"=enable; "0"=disable
 * mm_player_set_opts("video_rotate", "", AV_ROTATE_NONE);
 * mm_player_set_opts("video_ratio", "", AV_SCREEN_MODE);
 * mm_player_set_opts("audio_device", "", 0); -- "0"=panel; "2/3"=hdmi
 * mm_player_set_opts("audio_layout", "", AV_CH_LAYOUT_MONO); -- set audio layout
 * mm_player_set_opts("enable_scaler", "", 1); -- enable scaler module, such as: divp/vpe, "1"=enable; "0"=disable
 * mm_player_set_opts("resolution", "921600", 0); -- set the max resolution of video, 921600 = 1280 x 720
 * mm_player_set_opts("play_mode", "", AV_LOOP); -- set player mode, such as loop or once.
 */
int mm_player_set_opts(const char *key, const char *value, int flags);

/**
 * Get player real status.
 * Return value:
 * AV_ACODEC_ERROR  -- video deocder error
 * AV_VCODEC_ERROR  -- audio deocder error
 * AV_NOSYNC        -- audio and video is not sync
 * AV_READ_TIMEOUT  -- read data from network over time out
 * AV_NO_NETWORK    -- not find network
 * AV_INVALID_FILE  -- the file name or url is invalid
 * AV_NOTHING       -- player is normal
 * AV_PLAY_COMPLETE -- end of file
 * AV_PLAY_PAUSE    -- stoping player
 */
int mm_player_get_status(void);

/**
 * Enable/disable display if player is working. Clear screen if player is closed.
 * Please call the api after exit player to clear display.
 */
int mm_player_flush_screen(bool enable);

#ifdef __cplusplus
}
#endif // __cplusplu

#endif
