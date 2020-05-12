#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>



using namespace std;
class mainMem{
    public:
        mainMem();//
        int get_access();//
        int set_access();//
    private:
        int access;

}
class theCache {
    public:
        theCache(L1* l1, L2* l2,int lines_num, int block_size, int tag_size)
        int readLine(long int line);
        int writeLine(long int line);
    private:
        L1* l1_;
        L2* l2_;
        int lines_num_;
        int block_size_;
        int tag_size_;
}
class cache{
    public :
        cache(int num_ways,int lines_num, int block_size, int tag_size,int lru_size);//
        virtual int read_line (long int tag,long int set) = 0;
        virtual int write_line (long int tag,long int set) = 0;
        virtual int snoop(int set, int tag)=0;
        int get_miss();//
        int get_access();//
        int find_tag(int tag);
        int find_replace_tag(int tag);
        
    protected:
        //cache* another;
        int lines_num_;
        int block_size_;
        int tag_size_;
        int num_ways;
        vector <wayTable> table;
        //const int access_time;
        int miss;
        int access;
        int times_access;

}
class L1 : public cache{
    public:
        L1(int num_ways,int lines_num, int block_size, int tag_size,int lru_size);//
        int read_line (long int tag,long int set);
        int write_line (long int tag,long int set);
        int snoop(int set, int tag);
    
}
class L2 : public cache{
    public:
        L2(int num_ways,int lines_num, int block_size, int tag_size,int lru_size);//
        int read_line (long int tag,long int set);
        int write_line (long int tag,long int set);
        int snoop(int set, int tag);
    
}


class cacheLine {
    public:
        cacheLine(int block_size,int tag_size,int lru);//
        int get_tag();//
        bool get_dirty();//
        int get_lru();//
        void set_tag(int tag);//
        void set_lru(int change);//
        void set_dirty(bool change);//
        bool get_taken();//

    private:
        const int block_size_;
        const int tag_size_;
        const int max_lru_value_;
        bool taken_;
        bool dirty_;
        int lruBits_;
        int tag_;
}
class wayTable {
    public:
        wayTable(int lines_num,int block_size,int tag_size,int lru_size);//
        int get_tag(int line_num);//
        int get_lru(int line_num);//
        bool is_taken(int line_num);//
        void set_tag(int line_num,int tag);//
        void set_lru(int change, int line_num);//
        void set_dirty(bool change, int line_num);//
    private:
        int lines_number;
        vector <cacheLine> table;

}


//end of classes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//main memory functions
mainMem::mainMem():access(0){}
int mainMem::set_access(){
    access++;
}
int mainMem::get_access(){
    return access;
}


//theCache functions 
/*class theCache {
    public:
        theCache(L1* l1, L2* l2,int lines_num, int block_size, int tag_size)
        int readLine(long int line);
        int writeLine(long int line);
    private:
        L1* l1;
        L2* l2;
        int lines_num_;
        int block_size_;
        int tag_size_;
}*/

theCache::theCache(L1* l1, L2* l2,int lines_num, int block_size, int tag_size):l2_(l2),l1_(l1), lines_num_(lines_num), block_size_(block_size), tag_size_(tag_size){}
int theCache::readLine(long int line){
    int tag = line >> (32 - tag_size_);
    int set = (line >> block_size_)%lines_num;
    int return_tag1;
    int return_tag2;
    int found1 = l1_->readLine(tag,size);//0-SUCCESS 1-FAILURE;
    if (found1 == 0)return 0;//found the line , return success;
    int found2 = l2_->readLine(tag,set);//0-SUCCESS 1-FAILURE;//////!!!!!!
    if (found2 == 0){
        return_tag = l1_->findReplace (tag, set);//tag>0->dirty, need to replace in l2;//////!!!!!
        if (return_tag > 0){
            l2_->dirtify(return_tag);/////!!!!!
        }
        return 0;
    }
    else{
        return_tag2=l2_->findReplace(tag, set);
        return_tag1 = l1_->findReplace(tag,set);
        if 


    }



    
}










//cache functions
cache :: cache(int num_ways,int lines_num, int block_size, int tag_size,int lru_size):num_ways(num_ways),miss(0),access(0),lines_num_(lines_num),block_size_(block_size),tag_size_(tag_size), times_access(0){
    wayTable basic = wayTable(lines_num,block_size,tag_size,lru_size);
    for (int i=0;i<num_ways;i++)table.push_back(basic);
}
int cache::get_access(){
    return access;
}

int cache::get_miss(){
    return miss;
}

//caches l1 l2 functions

L1::L1(int num_ways,int lines_num, int block_size, int tag_size,int lru_size):cache(num_ways, lines_num, block_size, tag_size, lru_size){}
L2::L2(int num_ways,int lines_num, int block_size, int tag_size,int lru_size):cache(num_ways, lines_num, block_size, tag_size, lru_size){}


int L1::read_line(long int tag,long int set){
    // long int offset;
    // long int tag = line >>(32-tag_size_);
    // long int set = line >>(block_size_);

    int lru=-1;
    int free_way = -1;
    bool found = false;
    set = set % lines_num_;
    access++;
    int i=0;
    for (auto it = table.begin(); it!=table.end(); it++){//run over all the ways[set]
        if (it.get_tag(set) == tag) {
            lru = it.get_lru(set);
            found = true;
            break;
        }
        if (it.is_taken(set)==false){
            free_way = i;
            break;
        }
        i++;
    }
    if (found == true){
        for (auto it = table.begin(); it!=table.end(); it++){
            it.set_lru(lru, set);
        }
        return 0;//SUCCESS;
    }
    else {
        miss++;
        return 1;//FAILURE;
    }    
}

int L2::read_line(long int line){

}









//wayTable Functions
wayTable :: wayTable(int lines_num,int block_size,int tag_size,int lru_size){
    lines_number = lines_num;
    cacheLine basic = cacheLine(block_size,tag_size,lru_size);
    for (int i=0;i<lines_num;i++)table.push_back(basic);
}
int wayTable::get_tag(int line_num){
    return table[line_num].get_tag();
}
int wayTable::get_lru(int line_num){
    return table[line_num].get_lru();
}
bool wayTable::is_taken(int line_num){
    return table[line_num].get_taken();
}
void wayTable::set_tag(int line_num,int tag){
    table[line_num].set_tag(tag);
}
void wayTable::set_lru(int change, int line_num){
    table[line_num].set_lru(change);
}
void wayTable::set_dirty(bool change, int line_num){
    table[line_num].set_dirty(change);
}

//cacheLine Functions
cacheLine::cacheLine(int block_size,int tag_size,int lru):block_size_(block_size),tag_size_(tag_size),taken_(false),dirty_(false),lruBits_(0),tag_(0),max_lru_value_(pow(2,lru)){}
}

int cacheLine::get_tag(){return tag;}
bool cacheLine::get_dirty(){return dirty;}
int cacheLine::get_lru(){ return lruBits;}
bool cacheLine::get_taken(){return taken;}
void cacheLine::set_tag(int tag){
    tag_ = tag;
    lruBits_ = 0;
}
void cacheLine::set_lru(int change){
    //if (up==true){
        if (lruBits_<change) lruBits_++;
        if (lruBits_==change)lruBits_=0;
    //}
}
//start lru = 0;

void cacheLine::set_dirty(bool change){
    if (change == true )dirty_ = true;
}
