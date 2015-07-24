#ifndef INC_LSOUND_USERPLAYER_H__
#define INC_LSOUND_USERPLAYER_H__
/**
@file UserPlayer.h
@author t-sakai
@date 2015/07/06 create
*/
#include "lsound_api.h"

#if defined(LSOUND_API_WASAPI)
#include "Wasapi/UserPlayer.h"

#elif defined(LSOUND_API_OPENAL)
#include "OpenAL/UserPlayer.h"

#elif defined(LSOUND_API_OPENSL)
#include "OpenSL/UserPlayer.h"
#endif

#endif //INC_LSOUND_USERPLAYER_H__