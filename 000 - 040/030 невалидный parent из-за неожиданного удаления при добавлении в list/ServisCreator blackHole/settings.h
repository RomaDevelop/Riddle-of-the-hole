#ifndef SETTINGS_H
#define SETTINGS_H

struct Settings
{
	inline static bool logDoEvrythingCommands = false;
	inline static bool notSaveSettings = false;
	inline static bool warningNotPassedFilterFrames = false;
	inline static bool logParamChanges = true;
	inline static bool logFrameSent = true;
	inline static bool logFrameGet = true;

	inline static void SetDefault()
	{
		logDoEvrythingCommands = false;
		notSaveSettings = false;
		warningNotPassedFilterFrames = false;
		logParamChanges = true;
		logFrameSent = true;
		logFrameGet = true;
	};
};

#endif // SETTINGS_H
