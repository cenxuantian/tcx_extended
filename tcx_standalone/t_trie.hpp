#pragma once
#include <string>
#include <string.h>
#include <type_traits>
#include <optional>
#include <vector>

namespace tcx
{
template<bool view = false>
class Trie_26{
public:
    friend class Trie_26<!view>;
    struct Node{
        Node* next[26];
        bool is_end;
    };

private:
    Node* root_;

    Trie_26(Node* _node):root_(_node){}

    void free_all(Node* node){
        for(int i = 0; i< 26;i++){
            if(node->next[i]){
                free_all(node->next[i]);
                node->next[i]=nullptr;
            }
        }
        delete node;
    }

    Node* create_node(){
        Node* new_node = new Node;
        ::memset(new_node->next,0,26*sizeof(Node*));
        new_node->is_end = false;
        return new_node;
    }
    
    int get_index(char i) noexcept{
        if(i >= 'a' && i <='z'){
            char diff = i-'a';
            return diff;
        }else if (i >='A' && i <='Z'){
            char diff = i-'A';
            return diff;
        }
        return -1;
    }

    void get_all(std::vector<std::string>& cntr, std::string const& curstr, Node* cur_node){
        if(cur_node->is_end){
            cntr.emplace_back(curstr);
            return;
        }else{
            for(int i = 0 ; i < 26 ; i++){
                if(cur_node->next[i]){
                    std::string new_str = curstr+(char)('a'+i);
                    get_all(cntr,new_str,cur_node->next[i]);
                }
            }
            return;
        }
    }

public:
    Trie_26():root_(nullptr){}
    Trie_26(const Trie_26&)=delete;
    Trie_26(Trie_26&& other)noexcept:root_(other.root_){other.root_ = nullptr;}
    Trie_26& operator=(const Trie_26&)=delete;
    Trie_26& operator=(Trie_26&& other){root_ = other.root_;other.root_=nullptr;}
    ~Trie_26(){
        if constexpr (!view){
            if(root_)free_all(root_);
            root_=nullptr;
        }
    }
    
    void push(std::string const& word){
        if(!root_) root_= create_node();
        Node* cur = root_;
        for(char i : word){
            int index = get_index(i);
            if(index==-1)goto end;

            if(cur->next[index]){
                cur = cur->next[index];
            }else{
                Node* new_node = create_node();
                cur->next[index]= new_node;
                cur = new_node;
            }
        }

        end:
        cur->is_end = true;
    }

    std::optional<Trie_26<true>> sub_trie(std::string const& prefix){
        Node* cur = root_;
        for(char i : prefix){
            int index = get_index(i);
            if(index==-1) return {};

            if(!cur) return {};
            if(!cur->next[index]) return {};
            else{
                cur = cur->next[index];
            }
        }
        return {Trie_26<true>((Trie_26<true>::Node*)cur)};
    }

    bool search(std::string const& prefix){
        std::optional<Trie_26<true>> trie_sub = sub_trie(prefix);
        return trie_sub.has_value();
    }

    bool has_word(std::string const& word){
        std::optional<Trie_26<true>> trie_sub = sub_trie(word);
        if(!trie_sub.has_value())return false;
        if(!trie_sub->root_)return false;
        if(!trie_sub->root_->is_end)return false;
        return true;
    }

    std::vector<std::string> all(){
        std::vector<std::string> res;
        Node* cur_node = root_;
        if(!cur_node)return res;
        std::string curstr="";
        get_all(res,curstr,cur_node);
        return res;
    }

};


} // namespace tcx
