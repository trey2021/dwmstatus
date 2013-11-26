#include "alsavolume.h"
#include <alsa/asoundlib.h>

#define MIXER_NAME "Master"
#define GFX_CARD "default"
#define VOLUME_ERR -1

long alsa_get_volume()
{
    snd_mixer_t *handle;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    int mix_index = 0;
    long result = 0;

    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, MIXER_NAME);

    if ((snd_mixer_open(&handle, 0)) >= 0)
    {
        if (snd_mixer_attach(handle, GFX_CARD) >= 0)
        {
            if (snd_mixer_selem_register(handle, NULL, NULL) >= 0)
            {
                if (snd_mixer_load(handle) >= 0)
                {
                    elem = snd_mixer_find_selem(handle, sid);
                    
                    if (elem)
                    {
                        if (snd_mixer_selem_get_playback_volume(elem, 0, &result) >= 0)
                        {
                            return result;
                        }
                    }
                }
            }
        }
    }

    snd_mixer_close(handle);
    return VOLUME_ERR;
}

