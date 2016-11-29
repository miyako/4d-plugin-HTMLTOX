#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>

#include <wkhtmltox/pdf.h>
#include <wkhtmltox/image.h>

#include <string>
#include <vector>

#include "wkhtmltox_4d.h"

int pid = 0;

int process_4d = 0;
int process_os = 0;

void status(CFStringRef status);

#pragma mark -

Boolean my_process(CFDictionaryRef userInfo)
{
	if(CFDictionaryContainsKey(userInfo, USERINFOKEY_PID)
		 && CFDictionaryContainsKey(userInfo, USERINFOKEY_PID_OS))
	{
		int process_4d_i = 0, process_os_i = 0;
		CFNumberRef process_4d_n, process_os_n;
		
		process_4d_n = (CFNumberRef)CFDictionaryGetValue(userInfo, USERINFOKEY_PID);
		if(CFNumberGetValue(process_4d_n, kCFNumberIntType, &process_4d_i))
		{
			if(process_4d_i == process_4d)
			{
				process_os_n = (CFNumberRef)CFDictionaryGetValue(userInfo, USERINFOKEY_PID_OS);
				if(CFNumberGetValue(process_os_n, kCFNumberIntType, &process_os_i))
				{
					return (process_os_i == process_os);
				}
			}
		}
	}
	return false;
}

Boolean my_command(CFDictionaryRef userInfo, CFStringRef value)
{
	if(CFDictionaryContainsKey(userInfo, USERINFOKEY_COMMAND))
	{
		CFStringRef keyValue = (CFStringRef)CFDictionaryGetValue(userInfo, USERINFOKEY_COMMAND);
		return (kCFCompareEqualTo == CFStringCompare(keyValue, value, kCFCompareCaseInsensitive));
	}
	return false;
}

#pragma mark -

void process_notification(CFNotificationCenterRef center,
													void *observer,
													CFStringRef name,
													const void *object,
													CFDictionaryRef userInfo)
{
	if(my_process(userInfo))
	{
		if(my_command(userInfo, CFSTR("terminate")))
		{
			
			CFRunLoopStop(CFRunLoopGetCurrent());	//exit CFRunLoopRun
			
		}else if(my_command(userInfo, CFSTR("run")))
		{
			CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
																															&kCFTypeDictionaryKeyCallBacks,
																															&kCFTypeDictionaryValueCallBacks);
			
			CFArrayRef options = 0;
			CFArrayRef sources = 0;
			
			/* format */
			
			int fmt = HTMLTOX_Format_PDF;
			Boolean isOutputPDF = true;
			if(CFDictionaryContainsKey(userInfo, USERINFOKEY_FORMAT))
			{
				CFNumberRef format = (CFNumberRef)CFDictionaryGetValue(userInfo, USERINFOKEY_FORMAT);
				CFNumberGetValue(format, kCFNumberIntType, &fmt);
			}
			switch (fmt)
			{
				case HTMLTOX_Format_PNG:
				case HTMLTOX_Format_JPG:
				case HTMLTOX_Format_BMP:
				case HTMLTOX_Format_SVG:
					isOutputPDF = false;
					break;
			};
			
			wkhtmltopdf_global_settings *pdf_global_settings = 0;
			wkhtmltoimage_global_settings *image_global_settings = 0;
			if(isOutputPDF)
			{
				pdf_global_settings = wkhtmltopdf_create_global_settings();
				switch (fmt)
				{
					case HTMLTOX_Format_PDF:
						wkhtmltopdf_set_global_setting(pdf_global_settings, "outputFormat", "pdf");
						break;
					case HTMLTOX_Format_PS:
						wkhtmltopdf_set_global_setting(pdf_global_settings, "outputFormat", "ps");
						break;
					default:
						break;
				};
			}else
			{
				image_global_settings = wkhtmltoimage_create_global_settings();
				
				switch (fmt)
				{
					case HTMLTOX_Format_PNG:
						wkhtmltoimage_set_global_setting(image_global_settings, "fmt", "png");
						break;
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
						break;
				};
			}
			
			/* object settings */
			std::vector<wkhtmltopdf_object_settings *>object_settings;
			
			/* converter */
			
			wkhtmltopdf_converter *pdf_converter = 0;
			wkhtmltoimage_converter *image_converter = 0;
			
			if(isOutputPDF)
			{
				pdf_converter = wkhtmltopdf_create_converter(pdf_global_settings);
			}
			
			if(CFDictionaryContainsKey(userInfo, USERINFOKEY_SOURCES))
			{
				sources = (CFArrayRef)CFDictionaryGetValue(userInfo, USERINFOKEY_SOURCES);
				
				if(CFDictionaryContainsKey(userInfo, USERINFOKEY_OPTIONS))
				{
					options = (CFArrayRef)CFDictionaryGetValue(userInfo, USERINFOKEY_OPTIONS);
					
					/* global settings */
					
					for(CFIndex i = 1; i < CFArrayGetCount(options); ++i)
					{
						CFDictionaryRef option = (CFDictionaryRef)CFArrayGetValueAtIndex(options, i);
						CFStringRef name = (CFStringRef)CFDictionaryGetValue(option, CFSTR("name"));
						CFStringRef value = (CFStringRef)CFDictionaryGetValue(option, CFSTR("value"));
						const char *_name = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
						const char *_value = CFStringGetCStringPtr(value, kCFStringEncodingUTF8);
						
						if(_name && _value)
						{
							CFRange range =CFStringFind(name, CFSTR(":"), 0);
							if(range.location == kCFNotFound)
							{
								if(isOutputPDF)
								{
									wkhtmltopdf_set_global_setting(pdf_global_settings, _name, _value);
								}else
								{
									wkhtmltoimage_set_global_setting(image_global_settings, _name, _value);
								}
							}
						}
					}
					
					for(CFIndex i = 1; i < CFArrayGetCount(sources); ++i)
					{
						CFDictionaryRef source = (CFDictionaryRef)CFArrayGetValueAtIndex(sources, i);
						CFStringRef path = (CFStringRef)CFDictionaryGetValue(source, CFSTR("data"));
						CFStringRef html = (CFStringRef)CFDictionaryGetValue(source, CFSTR("html"));
						const char *src = CFStringGetCStringPtr(path, kCFStringEncodingUTF8);
						
						if(isOutputPDF)
						{
							wkhtmltopdf_object_settings *pdf_object_settings = wkhtmltopdf_create_object_settings();
							object_settings.push_back(pdf_object_settings);
							
							for(CFIndex j = 1; j < CFArrayGetCount(options); ++j)
							{
								CFDictionaryRef option = (CFDictionaryRef)CFArrayGetValueAtIndex(options, j);
								CFStringRef name = (CFStringRef)CFDictionaryGetValue(option, CFSTR("name"));
								CFStringRef value = (CFStringRef)CFDictionaryGetValue(option, CFSTR("value"));
								
								CFRange range =CFStringFind(name, CFSTR(":"), 0);
								if(range.location != kCFNotFound)
								{
									CFStringRef __name = CFStringCreateWithSubstring(kCFAllocatorDefault, name, CFRangeMake(range.location+1, CFStringGetLength(name) - range.location+1));
									if(name)
									{
										const char *_name = CFStringGetCStringPtr(__name, kCFStringEncodingUTF8);
										const char *_value = CFStringGetCStringPtr(value, kCFStringEncodingUTF8);
										if(_name && _value)
										{
											wkhtmltopdf_set_object_setting(pdf_object_settings, _name, _value);
										}
										CFRelease(__name);
									}
								}
								
							}//options
							
							if(src)
							{
								if(kCFCompareEqualTo == CFStringCompare(html, CFSTR("YES"), kCFCompareCaseInsensitive))
								{
									wkhtmltopdf_add_object(pdf_converter, pdf_object_settings, src);
								}else{
									wkhtmltopdf_set_object_setting(pdf_object_settings, "page", src);
									wkhtmltopdf_add_object(pdf_converter, pdf_object_settings, NULL);
								}
		
							}//src
							
						}else
						{
							if(kCFCompareEqualTo == CFStringCompare(html, CFSTR("YES"), kCFCompareCaseInsensitive))
							{
								image_converter = wkhtmltoimage_create_converter(image_global_settings, src);
							}else{
								wkhtmltoimage_set_global_setting(image_global_settings, "in", src);
								image_converter = wkhtmltoimage_create_converter(image_global_settings, NULL);
							}
							if(wkhtmltoimage_convert(image_converter))
							{
								const unsigned char *bytes;
								long len = wkhtmltoimage_get_output(image_converter, &bytes);
								CFDataRef data = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)bytes, len);
								CFDictionarySetValue(dict, CFSTR("data"), data);
								CFRelease(data);
							}
							wkhtmltoimage_destroy_converter(image_converter);
							break;
						}
						
					}//sources
				}//USERINFOKEY_OPTIONS
				
				if(isOutputPDF)
				{
					if(wkhtmltopdf_convert(pdf_converter))
					{
						const unsigned char *bytes;
						long len = wkhtmltopdf_get_output(pdf_converter, &bytes);
						CFDataRef data = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)bytes, len);
						CFDictionarySetValue(dict, CFSTR("data"), data);
						CFRelease(data);
					}
				}
		
			}	//USERINFOKEY_SOURCES
			
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
			
			CFNumberRef process_4d_n = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &process_4d);
			CFNumberRef process_os_n = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &process_os);
			
			CFDictionarySetValue(dict, USERINFOKEY_PID, process_4d_n);
			CFDictionarySetValue(dict, USERINFOKEY_PID_OS, process_os_n);
			
			CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
																					 htmltox_notification_name,
																					 NULL, dict, true);
			
			CFRelease(process_os_n);
			CFRelease(process_4d_n);
			CFRelease(dict);
		}
	}
}

void status(CFStringRef status)
{
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
																													&kCFTypeDictionaryKeyCallBacks,
																													&kCFTypeDictionaryValueCallBacks);
	
	CFNumberRef process_4d_n = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &process_4d);
	CFNumberRef process_os_n = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &process_os);
	
	CFDictionarySetValue(dict, USERINFOKEY_PID, process_4d_n);
	CFDictionarySetValue(dict, USERINFOKEY_PID_OS, process_os_n);
	CFDictionarySetValue(dict, USERINFOKEY_STATUS, status);
	
	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
																			 htmltox_notification_name,
																			 NULL, dict, true);
	
	CFRelease(process_os_n);
	CFRelease(process_4d_n);
	CFRelease(dict);
}

#pragma mark -

void listen_start()
{
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
																	NULL,	//this parameter may be NULL
																	process_notification,
																	htmltox4d_notification_name,	//this value must not be NULL
																	NULL,	//if NULL, callback is called when a notification named name is posted by any object
																	CFNotificationSuspensionBehaviorDeliverImmediately);	//irst flush any queued notifications

	status(CFSTR("OK"));
}

void listen_stop()
{
	CFNotificationCenterRemoveObserver(CFNotificationCenterGetDistributedCenter(),
																		 NULL,
																		 htmltox4d_notification_name,
																		 NULL);
}

#pragma mark -

int main(int argc, const char * argv[])
{
	for(unsigned int i=1; i<(argc-1); ++i)
	{
		if(!strcmp("pid", argv[i]))
		{
			process_4d = atoi(argv[i+1]);
		}else if(!strcmp("pidos", argv[i]))
		{
			process_os = atoi(argv[i+1]);
		}
	}
	
	if((process_4d) && (process_os))
	{
		wkhtmltopdf_init(0);
	
		listen_start();
		
		CFRunLoopRun();
		
		listen_stop();
		
		wkhtmltopdf_deinit();
	}else{
		status(CFSTR("KO"));	//just to unfreeze the caller
	}
	
	return 0;
}