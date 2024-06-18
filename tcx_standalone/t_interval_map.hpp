#pragma once

#include <map>

// typename K can does not implement any operations other than operator<
#define KEY_EQUAL(_upper_found_keyEnd,_input_keyEnd) (!(_upper_found_keyEnd<_input_keyEnd))
#define GET_ITEM_BEFORE(_map,_iter) {\
    if(_iter== _map.end())_iter = --_map.end();\
    else{\
        if(_iter==_map.begin()) _iter=_map.end();\
        else --_iter;\
    }}(void)0
        
namespace tcx{

template<typename K, typename V>
class IntervalMap {
	V m_valBegin;
	std::map<K,V> m_map;
public:
	// constructor associates whole range of K with val
	IntervalMap(V const& val)
	: m_valBegin(val)
	{}

	void emplace( K const& keyBegin, K const& keyEnd, V const& val ) {

        if(!( keyBegin < keyEnd )) return;  // K::operator< // ref: If !( keyBegin < keyEnd ), this designates an empty interval

        if(m_map.empty()){   // ref: the first entry in m_map must not contain the same value as m_valBegin 
            if(val == m_valBegin){// V::operator==
                return;
            }else{
                m_map.emplace(keyBegin,val);
                m_map.emplace(keyEnd,m_valBegin);
                return;
            }
        }

        // get the interval border
        auto l_pos = m_map.upper_bound(keyBegin);
        auto r_pos = m_map.upper_bound(keyEnd);
        GET_ITEM_BEFORE(m_map,l_pos);
        GET_ITEM_BEFORE(m_map,r_pos);

        if(l_pos == m_map.end() && r_pos == m_map.end()){// assigned interval is on the left of all intervels
            m_map.emplace(keyBegin,val);
            m_map.emplace(keyEnd,m_valBegin);
            while(m_map.begin()->second == m_valBegin) m_map.erase(m_map.begin());
            return;
        }else if(l_pos == m_map.end() && r_pos != m_map.end()){
            if(!KEY_EQUAL(r_pos->first,keyEnd)){  // equal to r_pos->first != keyEnd // border different // K::operator<
                m_map.emplace(keyEnd,r_pos->second);
                m_map.emplace(keyBegin,val);
            } else m_map.emplace(keyBegin,val);// r border is the same
            l_pos = m_map.begin();
            goto erase;
        }

        // l_pos and r_pos found
        if(KEY_EQUAL(l_pos->first,keyBegin) && KEY_EQUAL(r_pos->first,keyEnd)){
            l_pos->second = val;
        }
        else if(KEY_EQUAL(l_pos->first,keyBegin) && !KEY_EQUAL(r_pos->first,keyEnd)){
            m_map.emplace(keyEnd,r_pos->second);
            l_pos->second = val;
        }else if(!KEY_EQUAL(l_pos->first,keyBegin) && KEY_EQUAL(r_pos->first,keyEnd)){
            m_map.emplace(keyBegin,val);
            ++l_pos;
        }else{
            m_map.emplace(keyEnd,r_pos->second);
            m_map.emplace(keyBegin,val);
            ++l_pos;
        }

        erase:
        // erase all previous value between keyBegin and keyEnd (not included) // merge
        
        auto befor_l_pos = l_pos; --befor_l_pos;
        if(befor_l_pos!=m_map.end()) if(befor_l_pos->second ==val) l_pos=befor_l_pos;
        const V& l_v_ref = l_pos->second;
        ++l_pos;
        while(l_pos->first < keyEnd){
            l_pos = m_map.erase(l_pos);
        };
        if(l_pos->second == l_v_ref) m_map.erase(l_pos);
        while(m_map.begin()->second == m_valBegin) m_map.erase(m_map.begin());
        return;
	}

	V const& operator[]( K const& key ) const {
		auto it=m_map.upper_bound(key);
		if(it==m_map.begin()) {
			return m_valBegin;
		} else {
			return (--it)->second;
		}
	}
};

}

#undef KEY_EQUAL
#undef GET_ITEM_BEFORE