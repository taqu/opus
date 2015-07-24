#ifndef INC_LSOUND_CONTEXT_H__
#define INC_LSOUND_CONTEXT_H__
/**
@file Context.h
@author t-sakai
@date 2015/07/06 create
*/
#include "lsound_api.h"

#if defined(LSOUND_API_WASAPI)
#include "Wasapi/Context.h"

#elif defined(LSOUND_API_OPENAL)
#include "OpenAL/Context.h"

#elif defined(LSOUND_API_OPENSL)
#include "OpenSL/Context.h"
#endif

#endif //INC_LSOUND_CONTEXT_H__
