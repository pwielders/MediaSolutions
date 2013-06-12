void *__cdecl memrcpy(void * _Dst, const void * _Src, size_t _MaxCount)
{
	unsigned char* destination = static_cast<unsigned char *>(_Dst) + _MaxCount;
	const unsigned char* source = static_cast<const unsigned char *>(_Src) + _MaxCount;

	while (_MaxCount)
	{
		*destination-- = *source--;
		--_MaxCount;
	}

	return (destination);
}
