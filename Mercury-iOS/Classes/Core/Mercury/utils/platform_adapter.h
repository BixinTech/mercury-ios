#pragma once
#ifdef __APPLE__
#if __cplusplus < 201703L
#define weak_from_this()\
shared_from_this()
#endif
#endif
