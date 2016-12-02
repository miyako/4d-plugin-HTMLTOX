#ifndef wkhtmltox_4d_h
#define wkhtmltox_4d_h
#define HTMLTOX_CONNECTION_NAME @"wkhtmltox-4d-connection"

@interface HTMLTOX : NSObject

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

#define HTMLTOX_Format_PDF 0
#define HTMLTOX_Format_PS 1
#define HTMLTOX_Format_PNG 2
#define HTMLTOX_Format_JPG 3
#define HTMLTOX_Format_BMP 4
#define HTMLTOX_Format_SVG 5

#endif /* wkhtmltox_4d_h */