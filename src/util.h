#ifndef _UTIL_H_
#define _UTIL_H_

template<class InputIt1, class InputIt2>
bool intersect(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2)
{
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2) {
			++first1;
			continue;
		}
		if (*first2 < *first1) {
			++first2;
			continue;
		}
		return true;
	}
	return false;
}


template<class T, class U>
U tryAtMap(const std::map<T, U> map, const T& key, const U& def) {
	auto iter = map.find(key);
	if (iter != map.end()) return iter->second;
	return def;
}
#endif