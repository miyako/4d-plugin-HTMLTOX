#ifndef wkhtmltox_4d_h
#define wkhtmltox_4d_h

#define TEST_WIN_ON_MAC 0

#if TEST_WIN_ON_MAC | !__APPLE__
#include "cJSON.h"
#endif

#ifdef __APPLE__

#define HTMLTOX_CONNECTION_NAME @"wkhtmltox-4d-connection"

@interface HTMLTOX : NSObject

#if TEST_WIN_ON_MAC
- (NSData *)doIt:(NSData *)data;
#endif

- (NSData *)doIt:(NSArray *)sources
					format:(NSNumber *)format
				 options:(NSArray *)options;

- (void)quit;

- (NSData *)processPDF:(NSArray *)sources
								format:(NSNumber *)format
							 options:(NSArray *)options;

- (NSData *)processImage:(NSArray *)sources
									format:(NSNumber *)format
								 options:(NSArray *)options;

@end

#else
/* WINDOWS */
#define FLAG_IS_HELPER_READY L"wkhtmltox_4d_ready"
#define FLAG_HELPER_IDLE L"wkhtmltox_4d_idle"
#define FLAG_HELPER_BUSY L"wkhtmltox_4d_busy"
#define PARAM_IN L"wkhtmltox_4d_param_in"
#define PARAM_OUT L"wkhtmltox_4d_param_out"
#endif /* __APPLE__ */

#define HTMLTOX_Format_PDF 0
#define HTMLTOX_Format_PS 1
#define HTMLTOX_Format_PNG 2
#define HTMLTOX_Format_JPG 3
#define HTMLTOX_Format_BMP 4
#define HTMLTOX_Format_SVG 5

#endif /* wkhtmltox_4d_h */