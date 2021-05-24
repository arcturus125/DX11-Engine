#pragma once

#ifdef STRANGEENGINEMK3_EXPORTS
#define STRANGEENGINEMK3_API __declspec(dllexport)
#else
#define STRANGEENGINEMK3_API __declspec(dllimport)
#endif // STRANGEENGINE_EXPORTS


class StrangeEngine
{
public:
	STRANGEENGINEMK3_API void StartEngine();
};

