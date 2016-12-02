#import <Foundation/Foundation.h>
#import <wkhtmltox/pdf.h>
#import <wkhtmltox/image.h>
#import "wkhtmltox_4d.h"
#import <vector>

@implementation HTMLTOX : NSObject

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
								if([page intValue] == (i + 1))
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

#pragma mark -

int main(int argc, const char * argv[])
{
	wkhtmltopdf_init(0);

	HTMLTOX *htmltox = [[HTMLTOX alloc]init];
	NSConnection *connection = [[NSConnection alloc]init];
	[connection setRootObject:htmltox];
	
	if([connection registerName:HTMLTOX_CONNECTION_NAME])
	{
		NSLog(@"registered conenction: %@\n", HTMLTOX_CONNECTION_NAME);
	}else
	{
		NSLog(@"failed to register conenction: %@\n", HTMLTOX_CONNECTION_NAME);
	}
	
	CFRunLoopRun();

	[connection release];

	wkhtmltopdf_deinit();

	return 0;
}