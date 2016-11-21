#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <wkhtmltox/pdf.h>
#include <wkhtmltox/image.h>
#include <string>
#include "wkhtmltox_4d.h"

int pid = 0;

void post_notification(CFNotificationCenterRef center, const char *type, const int val, const SInt32 pid, const char *str);

#pragma mark callback function

void pdf_progress_cb(wkhtmltopdf_converter * converter, const int val)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "progress", val, pid, "");
}
void pdf_warning_cb(wkhtmltopdf_converter * converter, const char * str)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "warning", 0, pid, str);
}
void pdf_error_cb(wkhtmltopdf_converter * converter, const char * str)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "error", 0, pid, str);
}
void pdf_finished_cb(wkhtmltopdf_converter * converter, const int val)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "finished", val, pid, "");
}

#pragma mark -

void image_progress_cb(wkhtmltoimage_converter * converter, const int val)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "progress", val, pid, "");
}
void image_warning_cb(wkhtmltoimage_converter * converter, const char * str)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "warning", 0, pid, str);
}
void image_error_cb(wkhtmltoimage_converter * converter, const char * str)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "error", 0, pid, str);
}
void image_finished_cb(wkhtmltoimage_converter * converter, const int val)
{
	post_notification(CFNotificationCenterGetDistributedCenter(), "finished", val, pid, "");
}

#pragma mark -

void post_notification(CFNotificationCenterRef center, const char *type, const int val, const SInt32 pid, const char *str) {

	CFStringRef v1 = CFStringCreateWithCString(kCFAllocatorDefault, type, kCFStringEncodingUTF8);
	CFNumberRef v2 = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &val);
	CFNumberRef v3 = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid);
	CFStringRef v4 = CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingUTF8);
	
	const void *keys[]		=	{CFSTR("type"), CFSTR("val"), CFSTR("pid"), CFSTR("str")};
	const void *values[]	=	{v1, v2, v3, v4};
	
	CFDictionaryRef userInfo = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 4, NULL, NULL);
	
	CFNotificationCenterPostNotification(center,
																			 htmltox_notification_name,
																			 NULL, userInfo, true);
	
	CFRelease(userInfo);
}

int main(int argc, const char * argv[]) {
	
	unsigned int last = argc-1;
	unsigned int secondlast = last-1;
	int fmt = HTMLTOX_Format_PDF;
	bool isOutputPDF = true;
	
	for(unsigned int i=1; i<argc ;++i)
	{
		if(!strcmp("fmt", argv[i]) && i<last){
			fmt = atoi(argv[i+1]);
			switch (fmt)
			{
				case HTMLTOX_Format_PNG:
				case HTMLTOX_Format_JPG:
				case HTMLTOX_Format_BMP:
				case HTMLTOX_Format_SVG:
					isOutputPDF = false;
					break;
			};
		}else if(!strcmp("pid", argv[i]) && i<last){
			pid = atoi(argv[i+1]);
			break;
		}
	}
	
	if(pid)
	{
		wkhtmltopdf_init(0);
		
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
		
		bool ready = false;
		int pageNum = 0;
		
		for(unsigned int i=1; i<argc ;++i)
		{
			if(!strcmp("arg", argv[i]) && i<secondlast){
				
				const char *arg_name = argv[i+1];
				const char *arg_value = argv[i+2];
				pageNum++;
				
				if(isOutputPDF)
				{
					if(strcspn(arg_name, ":") == strlen(arg_name))//is it a page specific option?
					{
						wkhtmltopdf_set_global_setting(pdf_global_settings, arg_name, arg_value);
					}
				}else{
					wkhtmltoimage_set_global_setting(image_global_settings, arg_name, arg_value);
				}
				break;
			}
		}
		
		wkhtmltopdf_converter *pdf_converter = 0;
		wkhtmltoimage_converter *image_converter = 0;
		
		if(isOutputPDF)
		{
			pdf_converter = wkhtmltopdf_create_converter(pdf_global_settings);
		}
		
		for(int i=1; i<argc ;++i)
		{
			if(!strcmp("src", argv[i]) && i<last){
				const char *src = argv[i+1];
				if(isOutputPDF)
				{
					ready = true;
					wkhtmltopdf_object_settings *pdf_object_settings = wkhtmltopdf_create_object_settings();
					
					for(unsigned int j=1; j<argc ;++j)
					{
						if(!strcmp("arg", argv[j]) && j<secondlast){
							
							const char *arg_name = argv[j+1];
							const char *arg_value = argv[j+2];
							
							if(isOutputPDF)
							{
								std::string page_arg_name(arg_name);
								size_t pos = page_arg_name.find_first_of((const char *)":");
								if(pos != std::string::npos)
								{
									std::string prefix = page_arg_name.substr(0, pos);
									if(atoi((const char *)prefix.c_str()) == pageNum)
									{
										std::string realOptionName = page_arg_name.substr(pos + 1);
										const char *object_arg_name = (const char *)realOptionName.c_str();
										if(strlen(object_arg_name))
										{
											wkhtmltopdf_set_object_setting(pdf_object_settings, object_arg_name, arg_value);
//											fprintf(stderr, "object setting:%s, %s page:%i\n",  object_arg_name, arg_value, pageNum);
										}
									}
								}else{
									if(strlen(arg_name))
									{
										wkhtmltopdf_set_global_setting(pdf_global_settings, arg_name, arg_value);
//										fprintf(stderr, "global setting:%s, %s\n",  arg_name, arg_value);
									}
								}
								
							}else{
								wkhtmltoimage_set_global_setting(image_global_settings, arg_name, arg_value);
							}
							j++;j++;
						}
					}

					wkhtmltopdf_set_object_setting(pdf_object_settings, "page", src);
					wkhtmltopdf_add_object(pdf_converter, pdf_object_settings, NULL);
					// can't destroy here!!!
					// wkhtmltopdf_destroy_object_settings(pdf_object_settings);
				}else{
					wkhtmltoimage_set_global_setting(image_global_settings, "in", src);
					image_converter = wkhtmltoimage_create_converter(image_global_settings, NULL);
					
					wkhtmltoimage_set_error_callback(image_converter,
																					 (wkhtmltoimage_str_callback)image_error_cb);
					wkhtmltoimage_set_warning_callback(image_converter,
																						 (wkhtmltoimage_str_callback)image_warning_cb);
					
					wkhtmltoimage_set_progress_changed_callback(image_converter,
																											(wkhtmltoimage_int_callback)image_progress_cb);
					
					wkhtmltoimage_set_finished_callback(image_converter,
																							(wkhtmltoimage_int_callback)image_finished_cb);
					
					if(wkhtmltoimage_convert(image_converter))
					{
						const unsigned char *bytes;
						long len = wkhtmltoimage_get_output(image_converter, &bytes);
						freopen(NULL, "wb", stdout);
						fwrite(bytes, sizeof(UInt8), len, stdout);
					}
					wkhtmltoimage_destroy_converter(image_converter);
					break;
				}
				i++;
			}
		}
		
		if(isOutputPDF && ready)
		{
			wkhtmltopdf_set_progress_changed_callback(pdf_converter,
																								(wkhtmltopdf_int_callback)pdf_progress_cb);
			
			wkhtmltopdf_set_finished_callback(pdf_converter,
																				(wkhtmltopdf_int_callback)pdf_finished_cb);
			
			wkhtmltopdf_set_warning_callback(pdf_converter,
																			 (wkhtmltopdf_str_callback)pdf_warning_cb);
			
			wkhtmltopdf_set_error_callback(pdf_converter,
																		 (wkhtmltopdf_str_callback)pdf_error_cb);
			
			if(wkhtmltopdf_convert(pdf_converter))
			{
				const unsigned char *bytes;
				long len = wkhtmltopdf_get_output(pdf_converter, &bytes);
				freopen(NULL, "wb", stdout);
				fwrite(bytes, sizeof(UInt8), len, stdout);
			}else
			{
			}
		}
		
		if(pdf_converter)
		{
			wkhtmltopdf_destroy_converter(pdf_converter);
		}
		
		if(pdf_global_settings)
		{
			wkhtmltopdf_destroy_global_settings(pdf_global_settings);
		}
		
		wkhtmltopdf_deinit();	

	}
	
	return 0;
}