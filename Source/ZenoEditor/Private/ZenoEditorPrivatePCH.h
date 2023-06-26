#pragma once

#define ZENO_EDITOR

#include "ZenoEditor.h"

#define LOG_INFO(FMT, ...) UE_LOG(ZenoEditor, Display, (FMT), ##__VA_ARGS__)
#define LOG_WARNING(FMT, ...) UE_LOG(ZenoEditor, Warning, (FMT), ##__VA_ARGS__)
#define LOG_ERROR(FMT, ...) UE_LOG(ZenoEditor, Error, (FMT), ##__VA_ARGS__)
