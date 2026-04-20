/********************************************************************************
  * @file    iol_log.h
  * @author  Sanjana S , Calixto Firmware Team
  * @version V1.0.0
  * @date    27-Feb-2026
  * @brief   This file contains all the functions prototypes for the DATA LINK LAYER OF IO-LINK.
  *
  ******************************************************************************
  * <h2><center>&copy; COPYRIGHT 2026 Calixto Systems Pvt Ltd</center></h2>
  ******************************************************************************
  */

#ifndef IOL_LOG_H_INCLUDED
#define IOL_LOG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/sys/printk.h>   /* printk() — always available */

/* Silence "set but not used" warnings for suppressed log arguments */
#define IGNORE(...)  do { (void)(__VA_ARGS__); } while (0)

#define IOL_LOG_LEVEL_NONE     0
#define IOL_LOG_LEVEL_ERROR    1
#define IOL_LOG_LEVEL_WARNING  2
#define IOL_LOG_LEVEL_INFO     3
#define IOL_LOG_LEVEL_DEBUG    4

#ifndef IOL_LOG_LEVEL
#  define IOL_LOG_LEVEL  IOL_LOG_LEVEL_DEBUG
#endif


#if IOL_LOG_LEVEL >= IOL_LOG_LEVEL_ERROR
#  define IOL_LOG_ERR(_fmt, ...)  printk("[IOL-ERR] " _fmt "\r\n", ##__VA_ARGS__)
#else
#  define IOL_LOG_ERR(...)  IGNORE(__VA_ARGS__)
#endif

#if IOL_LOG_LEVEL >= IOL_LOG_LEVEL_WARNING
#  define IOL_LOG_WRN(_fmt, ...)  printk("[IOL-WRN] " _fmt "\r\n", ##__VA_ARGS__)
#else
#  define IOL_LOG_WRN(...)  IGNORE(__VA_ARGS__)
#endif

#if IOL_LOG_LEVEL >= IOL_LOG_LEVEL_INFO
#  define IOL_LOG_INF(_fmt, ...)  printk("[IOL-INF] " _fmt "\r\n", ##__VA_ARGS__)
#else
#  define IOL_LOG_INF(...)  IGNORE(__VA_ARGS__)
#endif

#if IOL_LOG_LEVEL >= IOL_LOG_LEVEL_DEBUG
#  define IOL_LOG_DBG(_fmt, ...)  printk("[IOL-DBG] " _fmt "\r\n", ##__VA_ARGS__)
#else
#  define IOL_LOG_DBG(...)  IGNORE(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* IOL_LOG_H_INCLUDED */