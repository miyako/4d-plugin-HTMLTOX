/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : HTMLTOX
 #	author : miyako
 #	2016/11/21
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

#pragma mark -

#define HELPER_NAME @"htmltox4d"

namespace HTMLTOX4D
{
	CFNotificationCenterRef center = NULL;
	NSString *launchPath;
	
	typedef PA_long32 internal_process_number_t;
	typedef int external_process_number_t;
	
	external_process_number_t process_os;
	
	std::map<internal_process_number_t, NSTask *> processes;
	std::map<internal_process_number_t, NSData *> results;
	
	void clear_buffer(internal_process_number_t pid)
	{
		std::map<internal_process_number_t, NSData *>::iterator find;
		find = results.find(pid);
		
		if(find != results.end())
		{
			NSData *data = find->second;
			results.erase(find);
			[data release];
		}
	}
	
	void terminate_helper(internal_process_number_t pid)
	{
		std::map<internal_process_number_t, NSTask *>::iterator find;
		find = processes.find(pid);
		
		if(find != processes.end())
		{
			NSTask *task = find->second;
			
			if([task isRunning])
			{
				NSMutableDictionary *userInfo = [[NSMutableDictionary alloc]init];
				
				[userInfo setObject:@"terminate" forKey:@"command"];
				[userInfo setObject:[NSNumber numberWithInt:(int)pid] forKey:@"pid"];
				[userInfo setObject:[NSNumber numberWithInt:(int)HTMLTOX4D::process_os] forKey:@"pidos"];
				
				CFNotificationCenterPostNotification(center,
																						 htmltox4d_notification_name,
																						 NULL,
																						 (CFDictionaryRef)userInfo,
																						 true);
				[userInfo release];
				
				NSLog(@"%@:%i was terminated for process:%i", HELPER_NAME, [task processIdentifier], (int)pid);
			}
			
			processes.erase(find);
			[task release];
		}
	}
	
	void cleanup()
	{
		internal_process_number_t pid = PA_GetCurrentProcessNumber();
		terminate_helper(pid);
		clear_buffer(pid);
	}
	
	void resume_process(CFDictionaryRef userInfo)
	{
		if(CFDictionaryContainsKey(userInfo, USERINFOKEY_PID))
		{
			int local_pid = 0;
			CFNumberRef remote_pid = (CFNumberRef)CFDictionaryGetValue(userInfo, USERINFOKEY_PID);
			if(CFNumberGetValue(remote_pid, kCFNumberIntType, &local_pid))
			{
				PA_UnfreezeProcess(local_pid);
			}
			
		}
	}
	
	void set_data(CFDictionaryRef userInfo)
	{
		if(CFDictionaryContainsKey(userInfo, USERINFOKEY_PID)
			 && CFDictionaryContainsKey(userInfo, USERINFOKEY_PID_OS))
		{
			int process_4d_i = 0, process_os_i = 0;
			CFNumberRef process_4d_n, process_os_n;
			
			process_4d_n = (CFNumberRef)CFDictionaryGetValue(userInfo, USERINFOKEY_PID);
			if(CFNumberGetValue(process_4d_n, kCFNumberIntType, &process_4d_i))
			{
				process_os_n = (CFNumberRef)CFDictionaryGetValue(userInfo, USERINFOKEY_PID_OS);
				if(CFNumberGetValue(process_os_n, kCFNumberIntType, &process_os_i))
				{
					if(process_os_i == HTMLTOX4D::process_os)
					{
						if(CFDictionaryContainsKey(userInfo, USERINFOKEY_DATA))
						{
							std::map<internal_process_number_t, NSData *>::iterator find;
							find = results.find(process_4d_i);
							if(find == results.end())
							{
								NSData *data = [[NSData alloc]initWithData:(NSData *)CFDictionaryGetValue(userInfo, USERINFOKEY_DATA)];
								results.insert(std::pair<internal_process_number_t, NSData *>(process_4d_i, data));
							}else{
								NSData *data = find->second;
								[data release];
								find->second = [[NSData alloc]initWithData:(NSData *)CFDictionaryGetValue(userInfo, USERINFOKEY_DATA)];
							}
						}
					}
				}	
			}
		}
	}
	
	void process_notification(CFNotificationCenterRef center,
														void *observer,
														CFStringRef name,
														const void *object,
														CFDictionaryRef userInfo)
	{
		if(CFDictionaryContainsKey(userInfo, USERINFOKEY_DATA))
		{
			set_data(userInfo);
		}
		
		resume_process(userInfo);
	}
	

	
	NSTask *start_helper(internal_process_number_t currentProcessId)
	{
		NSTask *task = [[NSTask alloc]init];
		[task setLaunchPath:HTMLTOX4D::launchPath];
		
		NSMutableArray *arguments = [[NSMutableArray alloc]init];
		[arguments addObject:@"pid"];
		[arguments addObject:[NSString stringWithFormat:@"%i", (int)currentProcessId]];
		[arguments addObject:@"pidos"];
		[arguments addObject:[NSString stringWithFormat:@"%i", (int)HTMLTOX4D::process_os]];
		[task setArguments:arguments];
		[arguments release];
		
		[task launch];
		
		PA_FreezeProcess(currentProcessId);
		
		return task;
	}
	
	bool init()
	{
		internal_process_number_t currentProcessId = PA_GetCurrentProcessNumber();
		std::map<internal_process_number_t, NSTask *>::iterator find;
		find = processes.find(currentProcessId);
		
		if(find == processes.end())
		{
			NSTask *task = start_helper(currentProcessId);
			processes.insert(std::pair<internal_process_number_t, NSTask *>(currentProcessId, task));
			NSLog(@"%@:%i was launched for process:%i", HELPER_NAME, [task processIdentifier], (int)currentProcessId);
			return true;
		}else{
			NSTask *task = find->second;
			if([task isRunning])
			{
				NSLog(@"%@:%i is running for process:%i", HELPER_NAME, [task processIdentifier], (int)currentProcessId);
				return true;
			}else
			{
				[task release];
				task = start_helper(currentProcessId);
				find->second = task;
				NSLog(@"%@:%i was relaunched for process:%i", HELPER_NAME, [task processIdentifier], (int)currentProcessId);
				return true;
			}
		}
		
		return false;
	}
	
	void exec(ARRAY_TEXT &inObjects,
						C_LONGINT &inOutputFormat,
						ARRAY_TEXT &inOptionName,
						ARRAY_TEXT &inOptionValue,
						C_BLOB &returnValue)
	{
		internal_process_number_t currentProcessId = PA_GetCurrentProcessNumber();
		
		if(init())
		{
			NSMutableDictionary *userInfo = [[NSMutableDictionary alloc]init];
			
			[userInfo setObject:@"run" forKey:@"command"];
			[userInfo setObject:[NSNumber numberWithInt:(int)currentProcessId] forKey:@"pid"];
			[userInfo setObject:[NSNumber numberWithInt:(int)HTMLTOX4D::process_os] forKey:@"pidos"];
			
			NSMutableArray *sources = [[NSMutableArray alloc]init];
			
			for(NSUInteger i = 0; i < inObjects.getSize(); ++i){
			
				NSString *path = inObjects.copyUTF16StringAtIndex(i);
				
				if( [path hasPrefix:@"http://"]
					||[path hasPrefix:@"ftp://"]
					||[path hasPrefix:@"https://"]
					||[path hasPrefix:@"file://"]
					)
				{
					NSDictionary *source = [[NSDictionary alloc]initWithObjectsAndKeys:
					 path, @"data",
					 @"NO", @"html", nil];
					[sources addObject:source];
					[source release];
				}else{
					NSString *_path = inObjects.copyPathAtIndex(i);
					if([[NSFileManager defaultManager]fileExistsAtPath:_path])
					{
						NSDictionary *source = [[NSDictionary alloc]initWithObjectsAndKeys:
																		_path, @"data",
																		@"NO", @"html",
																		nil];
						[sources addObject:source];
						[source release];
					}else{
						NSDictionary *source = [[NSDictionary alloc]initWithObjectsAndKeys:
																		path, @"data",
																		@"YES", @"html",
																		nil];
						[sources addObject:source];
						[source release];
					}
					[_path release];
				}
				[path release];
			}
			
			[userInfo setObject:sources forKey:@"sources"];
			
			int outputFormat = inOutputFormat.getIntValue();
			[userInfo setObject:[NSNumber numberWithInt:outputFormat] forKey:@"fmt"];
			
			NSMutableArray *options = [[NSMutableArray alloc]init];
			
			size_t countOptions = inOptionName.getSize();
			if(countOptions == inOptionValue.getSize())
			{
				for(int i = 0; i < countOptions; ++i)
				{
					NSString *name = inOptionName.copyUTF16StringAtIndex(i);
					NSString *value = inOptionValue.copyUTF16StringAtIndex(i);
					NSDictionary *option = [[NSDictionary alloc]initWithObjectsAndKeys:
																	name, @"name",
																	value, @"value",
																	nil];
					[options addObject:option];
					[option release];
					[name release];
					[value release];
				}
			}
			
			[userInfo setObject:options forKey:@"options"];
			
			CFNotificationCenterPostNotification(center,
																					 htmltox4d_notification_name,
																					 NULL,
																					 (CFDictionaryRef)userInfo,
																					 true);
			[options release];
			[sources release];
			[userInfo release];
			
			PA_FreezeProcess(currentProcessId);
			
			std::map<internal_process_number_t, NSData *>::iterator find;
			find = results.find(currentProcessId);
			if(find != results.end())
			{
				NSData *data = find->second;
				returnValue.setBytes((const uint8_t *)[data bytes], [data length]);
			}
		}
	}
}

#pragma mark -

bool IsProcessOnExit()
{
	C_TEXT name;
	PA_long32 state, time;
	PA_GetProcessInfo(PA_GetCurrentProcessNumber(), name, &state, &time);
	CUTF16String procName(name.getUTF16StringPtr());
	CUTF16String exitProcName((PA_Unichar *)"$\0x\0x\0\0\0");
	return (!procName.compare(exitProcName));
}

void OnStartup()
{
	HTMLTOX4D::process_os = [[NSRunningApplication currentApplication]processIdentifier];
	HTMLTOX4D::center = CFNotificationCenterGetDistributedCenter();
	//a distributed notification center delivers notifications between applications
	
	if(HTMLTOX4D::center)
	{
		CFNotificationCenterAddObserver(HTMLTOX4D::center,
																		NULL,	//this parameter may be NULL
																		HTMLTOX4D::process_notification,
																		htmltox_notification_name,	//this value must not be NULL
																		NULL,	//if NULL, callback is called when a notification named name is posted by any object
																		CFNotificationSuspensionBehaviorDeliverImmediately);	//irst flush any queued notifications
	}
	
	NSBundle *bundle = [NSBundle bundleWithIdentifier:this_bundle_id];
	
	if(bundle)
	{
		HTMLTOX4D::launchPath = [[[bundle executablePath]stringByDeletingLastPathComponent]
														 stringByAppendingPathComponent:@"wkhtmltox-4d"];
	}
}

void OnCloseProcess()
{
	HTMLTOX4D::cleanup();
	
	if(IsProcessOnExit())
	{
		if(HTMLTOX4D::center)
		{
			CFNotificationCenterRemoveObserver(HTMLTOX4D::center,
																				 NULL,
																				 htmltox_notification_name,
																				 NULL);
			
		}
	}
}

#pragma mark -

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
		case kInitPlugin :
		case kServerInitPlugin :
			OnStartup();
			break;
			
		case kCloseProcess :
			OnCloseProcess();
			break;
			
// --- HTMLTOX

		case 1 :
			HTML_Convert(pResult, pParams);
			break;

	}
}

#pragma mark -

// ------------------------------------ HTMLTOX -----------------------------------

void HTML_Convert(sLONG_PTR *pResult, PackagePtr pParams)
{
	ARRAY_TEXT inObjects;
	C_LONGINT inOutputFormat;
	ARRAY_TEXT inOptionName;
	ARRAY_TEXT inOptionValue;
	C_BLOB returnValue;
	
	inObjects.fromParamAtIndex(pParams, 1);
	inOutputFormat.fromParamAtIndex(pParams, 2);
	inOptionName.fromParamAtIndex(pParams, 3);
	inOptionValue.fromParamAtIndex(pParams, 4);

	HTMLTOX4D::exec(inObjects, inOutputFormat, inOptionName, inOptionValue, returnValue);
	
	returnValue.setReturn(pResult);
}