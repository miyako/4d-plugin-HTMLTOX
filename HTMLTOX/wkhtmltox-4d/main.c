#ifdef __APPLE__
#import <Foundation/Foundation.h>
#import <wkhtmltox/pdf.h>
#import <wkhtmltox/image.h>
#import "wkhtmltox_4d.h"
#import <vector>
#else
#include <windows.h>
#include <wkhtmltox/pdf.h>
#include <wkhtmltox/image.h>
#include "wkhtmltox_4d.h"
#include <vector>
#include <iterator>
#ifndef uint8_t
#define uint8_t unsigned int
#endif
#endif

#if TEST_WIN_ON_MAC | !__APPLE__
#pragma mark split
#include <sstream>
template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

#pragma mark c

void processImage(cJSON *sources, int format, cJSON *options, std::vector<unsigned char> &buf)
{
	wkhtmltoimage_global_settings *image_global_settings = wkhtmltoimage_create_global_settings();
	
	switch (format)
	{
		case HTMLTOX_Format_JPG:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "jpg");
			break;
		case HTMLTOX_Format_BMP:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "bmp");
			break;
		case HTMLTOX_Format_SVG:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "svg");
			break;
		default:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "png");
			break;
	};
	
	/* global options */
	
	for(unsigned int i = 0; i < cJSON_GetArraySize(options); ++i)
	{
		cJSON *options_i = cJSON_GetArrayItem(options, i);
		std::string name = cJSON_GetObjectItem(options_i, "name")->valuestring;
		std::string value = cJSON_GetObjectItem(options_i, "value")->valuestring;
		
		if(name.length() && value.length())
		{
			std::vector<std::string>components = split(name, ':');
			
			if(components.size() == 1)
			{
				wkhtmltoimage_set_global_setting(image_global_settings, name.c_str(), value.c_str());
			}
		}
		
	}
	
	for(unsigned int i = 0; i < cJSON_GetArraySize(sources); ++i)
	{
		cJSON *sources_i = cJSON_GetArrayItem(sources, i);
		std::string data = cJSON_GetObjectItem(sources_i, "data")->valuestring;
		std::string type = cJSON_GetObjectItem(sources_i, "type")->valuestring;
		
		if(data.length() && type.length())
		{
			wkhtmltoimage_converter *image_converter = 0;
			if(type.compare("html") == 0)
			{
				image_converter = wkhtmltoimage_create_converter(image_global_settings, data.c_str());
			}else{
				wkhtmltoimage_set_global_setting(image_global_settings, "in", data.c_str());
				image_converter = wkhtmltoimage_create_converter(image_global_settings, NULL);
			}
			if(wkhtmltoimage_convert(image_converter))
			{
				const unsigned char *bytes;
				buf.resize(wkhtmltoimage_get_output(image_converter, &bytes));
				memcpy(&buf[0], bytes, buf.size());
			}
			wkhtmltoimage_destroy_converter(image_converter);
			
			break;
		}
		
	}
	
}

void processPDF(cJSON *sources, int format, cJSON *options, std::vector<unsigned char> &buf)
{
	wkhtmltopdf_global_settings *pdf_global_settings = wkhtmltopdf_create_global_settings();
	
	switch (format)
	{
		case HTMLTOX_Format_PS:
			wkhtmltopdf_set_global_setting(pdf_global_settings, "outputFormat", "ps");
			break;
		default:
			wkhtmltopdf_set_global_setting(pdf_global_settings, "outputFormat", "pdf");
			break;
	}
	
	std::vector<wkhtmltopdf_object_settings *>object_settings;
	
	wkhtmltopdf_converter *pdf_converter = wkhtmltopdf_create_converter(pdf_global_settings);
	
	/* global options */
	
	for(unsigned int i = 0; i < cJSON_GetArraySize(options); ++i)
	{
		cJSON *options_i = cJSON_GetArrayItem(options, i);
		std::string name = cJSON_GetObjectItem(options_i, "name")->valuestring;
		std::string value = cJSON_GetObjectItem(options_i, "value")->valuestring;
		
		if(name.length() && value.length())
		{
			std::vector<std::string>components = split(name, ':');
			
			if(components.size() == 1)
			{
				wkhtmltopdf_set_global_setting(pdf_global_settings, name.c_str(), value.c_str());
			}
		}
		
	}
	
	for(unsigned int i = 0; i < cJSON_GetArraySize(sources); ++i)
	{
		cJSON *sources_i = cJSON_GetArrayItem(sources, i);
		std::string data = cJSON_GetObjectItem(sources_i, "data")->valuestring;
		std::string type = cJSON_GetObjectItem(sources_i, "type")->valuestring;
		
		if(data.length() && type.length())
		{
			wkhtmltopdf_object_settings *pdf_object_settings = wkhtmltopdf_create_object_settings();
			object_settings.push_back(pdf_object_settings);
			
			//check for per-source option
			for(unsigned int j = 0; j < cJSON_GetArraySize(options); ++j)
			{
				cJSON *options_j = cJSON_GetArrayItem(options, j);
				std::string name = cJSON_GetObjectItem(options_j, "name")->valuestring;
				std::string value = cJSON_GetObjectItem(options_j, "value")->valuestring;
				
				if(name.length() && value.length())
				{
					std::vector<std::string>components = split(name, ':');
					
					if(components.size() == 2)
					{
						std::string page = components.at(0);
						
						if(atoi(page.c_str()) == i)
						{
							name = components.at(1);
							wkhtmltopdf_set_object_setting(pdf_object_settings, name.c_str(), value.c_str());
						}
					}
				}
				
			}//options
			if(type.compare("html") == 0)
			{
				wkhtmltopdf_add_object(pdf_converter, pdf_object_settings, data.c_str());
			}else{
				wkhtmltopdf_set_object_setting(pdf_object_settings, "page", data.c_str());
				wkhtmltopdf_add_object(pdf_converter, pdf_object_settings, NULL);
			}
		}
	}
	
	/* convert */
	
	if(wkhtmltopdf_convert(pdf_converter))
	{
		const unsigned char *bytes;
		buf.resize(wkhtmltopdf_get_output(pdf_converter, &bytes));
		memcpy(&buf[0], bytes, buf.size());
	}
	
	/* cleanup */
	
	for(std::vector<wkhtmltopdf_object_settings *>::iterator v = object_settings.begin(); v != object_settings.end(); ++v)
	{
		wkhtmltopdf_object_settings *pdf_object_settings = *v;
		wkhtmltopdf_destroy_object_settings(pdf_object_settings);
	}
	
	object_settings.clear();
	
	if(pdf_converter)
	{
		wkhtmltopdf_destroy_converter(pdf_converter);
	}
	
	if(pdf_global_settings)
	{
		wkhtmltopdf_destroy_global_settings(pdf_global_settings);
	}
}
#endif

#ifdef __APPLE__

#pragma mark obj-c

@implementation HTMLTOX : NSObject

#if TEST_WIN_ON_MAC

- (NSData *)doIt:(NSData *)data
{
	cJSON * root = cJSON_Parse((const char *)[data bytes]);
	
	cJSON *sources = cJSON_GetObjectItem(root, "sources");
	int format = cJSON_GetObjectItem(root, "format")->valueint;
	cJSON *options = cJSON_GetObjectItem(root, "options");

	NSLog(@"format: %i\n", format);
	
	NSData *result = [NSData data];
	
	std::vector<unsigned char> buf(0);
	
	switch (format)
	{
		case HTMLTOX_Format_PNG:
		case HTMLTOX_Format_JPG:
		case HTMLTOX_Format_BMP:
		case HTMLTOX_Format_SVG:
			processImage(sources, format, options, buf);
			result = [NSData dataWithBytes:(const void *)&buf[0] length:buf.size()];
			break;
		default:
			processPDF(sources, format, options, buf);
			result = [NSData dataWithBytes:(const void *)&buf[0] length:buf.size()];
			break;
	};
	
	cJSON_Delete(root);
	
	return result;
}
#endif

- (NSData *)doIt:(NSArray *)sources
					format:(NSNumber *)format
				 options:(NSArray *)options
{
//	NSLog(@"got message: %@\n", @"doIt");

	NSLog(@"format: %i\n", (int)[format intValue]);
	
	switch ([format intValue])
	{
		case HTMLTOX_Format_PNG:
		case HTMLTOX_Format_JPG:
		case HTMLTOX_Format_BMP:
		case HTMLTOX_Format_SVG:
			return [self processImage:sources format:format options:options];
			break;
		default:
			return [self processPDF:sources format:format options:options];
			break;
	};
}

- (void)quit
{
//	NSLog(@"got message: %@\n", @"quit");
	CFRunLoopStop(CFRunLoopGetCurrent());
}

- (NSData *)processPDF:(NSArray *)sources
								format:(NSNumber *)format
							 options:(NSArray *)options
{
	NSData *pdf = [NSData data];
	
	NSLog(@"global options: %@\n", [options description]);
	NSLog(@"sources: %@\n", [sources description]);
	
	wkhtmltopdf_global_settings *pdf_global_settings = wkhtmltopdf_create_global_settings();
	
	switch ([format intValue])
	{
		case HTMLTOX_Format_PS:
			wkhtmltopdf_set_global_setting(pdf_global_settings, "outputFormat", "ps");
			break;
		default:
			wkhtmltopdf_set_global_setting(pdf_global_settings, "outputFormat", "pdf");
			break;
	};
	
	std::vector<wkhtmltopdf_object_settings *>object_settings;
	
	wkhtmltopdf_converter *pdf_converter = wkhtmltopdf_create_converter(pdf_global_settings);
	
	/* global options */
	
	for(NSUInteger i = 0; i < [options count]; ++i)
	{
		NSDictionary *option = [options objectAtIndex:i];
		if(option)
		{
			NSString *name = [option valueForKey:@"name"];
			NSString *value = [option valueForKey:@"value"];
			if((name) && (value) && [name length] && [value length])
			{
				NSArray *components = [name componentsSeparatedByString:@":"];
				if([components count] == 1)
				{
					wkhtmltopdf_set_global_setting(pdf_global_settings, [name UTF8String], [value UTF8String]);
					NSLog(@"global option: %@, %@\n", name, value);
				}
			}
		}
	}
	
	for(NSUInteger i = 0; i < [sources count]; ++i)
	{
		NSDictionary *source = [sources objectAtIndex:i];
		if(source)
		{
			NSString *data = [source valueForKey:@"data"];
			NSString *type = [source valueForKey:@"type"];
			if((data) && (type) && [data length] && [type length])
			{
				wkhtmltopdf_object_settings *pdf_object_settings = wkhtmltopdf_create_object_settings();
				object_settings.push_back(pdf_object_settings);
				
				//check for per-source option
				for(NSUInteger j = 0; j < [options count]; ++j)
				{
					NSDictionary *option = [options objectAtIndex:j];
					if(option)
					{
						NSString *name = [option valueForKey:@"name"];
						NSString *value = [option valueForKey:@"value"];
						if((name) && (value) && [name length] && [value length])
						{
							NSArray *components = [name componentsSeparatedByString:@":"];
							if([components count] == 2)
							{
								NSString *page = [components objectAtIndex:0];
								if([page intValue] == i)
								{
									name = [components objectAtIndex:1];
									wkhtmltopdf_set_object_setting(pdf_object_settings, [name UTF8String], [value UTF8String]);
									NSLog(@"object option: %@, %@, %@\n", page, name, value);
								}
							}
						}
					}
				}//options
				if([type isEqualToString:@"html"])
				{
					wkhtmltopdf_add_object(pdf_converter, pdf_object_settings, [data UTF8String]);
				}else{
					wkhtmltopdf_set_object_setting(pdf_object_settings, "page", [data UTF8String]);
					wkhtmltopdf_add_object(pdf_converter, pdf_object_settings, NULL);
				}
			}
		}
	}//sources
	
	/* convert */
	
	if(wkhtmltopdf_convert(pdf_converter))
	{
		const unsigned char *bytes;
		long len = wkhtmltopdf_get_output(pdf_converter, &bytes);
		pdf = [NSData dataWithBytes:(const void *)bytes length:len];
	}
	
	/* cleanup */
	
	for(std::vector<wkhtmltopdf_object_settings *>::iterator v = object_settings.begin(); v != object_settings.end(); ++v)
	{
		wkhtmltopdf_object_settings *pdf_object_settings = *v;
		wkhtmltopdf_destroy_object_settings(pdf_object_settings);
	}
		
	object_settings.clear();
		
	if(pdf_converter)
	{
		wkhtmltopdf_destroy_converter(pdf_converter);
	}

	if(pdf_global_settings)
	{
		wkhtmltopdf_destroy_global_settings(pdf_global_settings);
	}

	return pdf;
}

- (NSData *)processImage:(NSArray *)sources
									format:(NSNumber *)format
								 options:(NSArray *)options
{
	NSData *image = [NSData data];
	
	wkhtmltoimage_global_settings *image_global_settings = wkhtmltoimage_create_global_settings();
	
	switch ([format intValue])
	{
		case HTMLTOX_Format_JPG:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "jpg");
			break;
		case HTMLTOX_Format_BMP:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "bmp");
			break;
		case HTMLTOX_Format_SVG:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "svg");
			break;
		default:
			wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "png");
			break;
	};
	
	NSLog(@"global options: %@\n", [options description]);
	NSLog(@"sources: %@\n", [sources description]);
	
	/* global options */
	
	for(NSUInteger i = 0; i < [options count]; ++i)
	{
		NSDictionary *option = [options objectAtIndex:i];
		if(option)
		{
			NSString *name = [option valueForKey:@"name"];
			NSString *value = [option valueForKey:@"value"];
			if((name) && (value) && [name length] && [value length])
			{
				NSArray *components = [name componentsSeparatedByString:@":"];
				if([components count] == 1)
				{
					wkhtmltoimage_set_global_setting(image_global_settings, [name UTF8String], [value UTF8String]);
					NSLog(@"global option: %@, %@\n", name, value);
				}
			}
		}
	}
	
	for(NSUInteger i = 0; i < [sources count]; ++i)
	{
		NSDictionary *source = [sources objectAtIndex:i];
		if(source)
		{
			NSString *data = [source valueForKey:@"data"];
			NSString *type = [source valueForKey:@"type"];
			if((data) && (type) && [data length] && [type length])
			{
				wkhtmltoimage_converter *image_converter = 0;
				if([type isEqualToString:@"html"])
				{
					image_converter = wkhtmltoimage_create_converter(image_global_settings, [data UTF8String]);
				}else{
					wkhtmltoimage_set_global_setting(image_global_settings, "in", [data UTF8String]);
					image_converter = wkhtmltoimage_create_converter(image_global_settings, NULL);
				}
				if(wkhtmltoimage_convert(image_converter))
				{
					const unsigned char *bytes;
					long len = wkhtmltoimage_get_output(image_converter, &bytes);
					image = [NSData dataWithBytes:(const void *)bytes length:len];
				}
				wkhtmltoimage_destroy_converter(image_converter);
				
				break;
			}
		}
	}//sources
	
	return image;
}

@end

#else
BOOL processRequest(HANDLE helper_c)
{
	BOOL exit = FALSE;
	
	std::string json;
	std::wstring uuid;
	
	DWORD inDataLen = 0, flagLen = 0;
	DWORD len = sizeof(flagLen) + sizeof(inDataLen);
	//first, create file mapping for header only (DWORD * 2)
	HANDLE fmIn = CreateFileMapping(
																	 INVALID_HANDLE_VALUE,
																	 NULL,
																	 PAGE_READWRITE,
																	 0, len,
																	 PARAM_IN);
	
	if(fmIn)
	{
		//read the header
		LPVOID bufIn = MapViewOfFile(fmIn, FILE_MAP_READ, 0, 0, len);
		if(bufIn)
		{
			unsigned char *p = (unsigned char *)bufIn;
			try
			{
				CopyMemory(&flagLen, p, sizeof(flagLen));
				p += sizeof(flagLen);
				CopyMemory(&inDataLen, p, sizeof(inDataLen));
			}
			catch(...)
			{
				
			}
			UnmapViewOfFile(bufIn);
		}//bufIn
		CloseHandle(fmIn);
	}//fmIn
	len = len + flagLen + inDataLen;
	//next, create file mapping for entire request
	fmIn = CreateFileMapping(
													 INVALID_HANDLE_VALUE,
													 NULL,
													 PAGE_READWRITE,
													 0, len,
													 PARAM_IN);
	if(fmIn)
	{
		LPVOID bufIn = MapViewOfFile(fmIn, FILE_MAP_READ, 0, 0, len);
		if(bufIn)
		{
			//receive request
			unsigned char *p = (unsigned char *)bufIn;
			p = p + sizeof(flagLen) + sizeof(inDataLen);
			if (flagLen)
			{
				std::vector<uint8_t>flagPtr(flagLen);
				CopyMemory(&flagPtr[0], p, flagLen);
				p += flagLen;
				uuid = std::wstring((const wchar_t *)&flagPtr[0], flagLen / (sizeof(wchar_t)));
			}
			if (inDataLen)
			{
				std::vector<uint8_t>inData(inDataLen);
				CopyMemory(&inData[0], p, inDataLen);
				json = std::string((const char *)&inData[0], inDataLen);
			}
			UnmapViewOfFile(bufIn);
		}//bufIn
		CloseHandle(fmIn);
	}//fmIn
	
	std::vector<unsigned char> buf(0);
	
	if(json.length() == 0)
	{
		exit = TRUE;
	}else
	{
		cJSON * root = cJSON_Parse((const char *)json.c_str());
		cJSON *sources = cJSON_GetObjectItem(root, "sources");
		int format = cJSON_GetObjectItem(root, "format")->valueint;
		cJSON *options = cJSON_GetObjectItem(root, "options");
		
		switch (format)
		{
			case HTMLTOX_Format_PNG:
			case HTMLTOX_Format_JPG:
			case HTMLTOX_Format_BMP:
			case HTMLTOX_Format_SVG:
				processImage(sources, format, options, buf);
				break;
			default:
				processPDF(sources, format, options, buf);
				break;
		}
		cJSON_Delete(root);
	}

	//send response
	DWORD outDataLen = buf.size();
	len = sizeof(outDataLen) + outDataLen;
	HANDLE fmOut = CreateFileMapping(
																	 INVALID_HANDLE_VALUE,
																	 NULL,
																	 PAGE_READWRITE,
																	 0, len,
																	 PARAM_OUT);
	if(fmOut)
	{
		LPVOID bufOut = MapViewOfFile(fmOut, FILE_MAP_WRITE, 0, 0, len);
		if(bufOut)
		{
			unsigned char *p = (unsigned char *)bufOut;
			try
			{
				CopyMemory(p, &outDataLen, sizeof(outDataLen));
				p += sizeof(outDataLen);
				if (outDataLen)
				{
					CopyMemory(p, &buf[0], outDataLen);
				}
			}
			catch(...)
			{
				
			}
			UnmapViewOfFile(bufOut);
		}//bufOut
	}//fmOut
	
	//send signal to caller
	HANDLE signal = OpenEvent(EVENT_ALL_ACCESS, FALSE, uuid.c_str());
	if(signal)
	{
		SetEvent(signal);
		CloseHandle(signal);
	}

	//wait for caller to receive data
	WaitForSingleObject(helper_c, INFINITE);
	ResetEvent(helper_c);

	if(fmOut) CloseHandle(fmOut);

	return exit;
}
#endif

#pragma mark main

int main(int argc, const char * argv[])
{
	wkhtmltopdf_init(0);

#ifdef __APPLE__
	HTMLTOX *htmltox = [[HTMLTOX alloc]init];
	NSConnection *connection = [[NSConnection alloc]init];
	[connection setRootObject:htmltox];
	
	if([connection registerName:HTMLTOX_CONNECTION_NAME])
	{
		NSLog(@"registered connection: %@\n", HTMLTOX_CONNECTION_NAME);
	}else
	{
		NSLog(@"failed to register connection: %@\n", HTMLTOX_CONNECTION_NAME);
	}
	
	CFRunLoopRun();

	[connection release];
	
#else
	
	HANDLE helper_c = CreateEvent(NULL, TRUE, FALSE, FLAG_HELPER_IDLE);
	if(helper_c)
	{
		HANDLE helper_s = CreateEvent(NULL, TRUE, FALSE, FLAG_HELPER_BUSY);
		if(helper_s)
		{
			HANDLE signal = CreateEvent(NULL, TRUE, TRUE, FLAG_IS_HELPER_READY);
			
			if(signal)
			{
				OutputDebugString(L"registered connection");
				
				BOOL exit = FALSE;
				while(!exit)
				{
					switch(WaitForSingleObject(helper_s, INFINITE))
					{
						case WAIT_OBJECT_0:
							ResetEvent(helper_s);
							exit = processRequest(helper_c);
							break;
						default :
							exit = TRUE;
							break;
					}
				}//!exit
				ResetEvent(signal);
				CloseHandle(signal);
			}else
			{
				OutputDebugString(L"failed to register connection");
			}
			CloseHandle(helper_s);
		}//helper_s
		CloseHandle(helper_c);
	}//helper_c
	
#endif
	
	wkhtmltopdf_deinit();

	return 0;
}