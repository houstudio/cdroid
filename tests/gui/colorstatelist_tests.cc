#include <gtest/gtest.h>
#include <cdroid.h>

using namespace cdroid;

class COLORSTATESET:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
   int index(const std::vector<std::vector<int>>&states,const std::vector<int>&state){
       for(int i=0;i<states.size();i++)
           if(StateSet::stateSetMatches(states[i],state))
               return i;
       return -1;
   }
};
   
TEST_F(COLORSTATESET,set){
   std::vector<int> v1={4};
   std::vector<int> v2={4,5};
   int idx=StateSet::stateSetMatches(v1,v2);
   printf("idx=%d\r\n",idx);
}
TEST_F(COLORSTATESET,color){
    unsigned int cc= Color::parseColor("#ff112233");
    ASSERT_EQ(cc,0xFF112233);
    printf("color=0x%x\r\n",cc);
}
TEST_F(COLORSTATESET,match){
    std::vector<int>stateEmpty;
    std::vector<int>state1={StateSet::PRESSED,StateSet::FOCUSED,StateSet::FOCUSED};
    std::vector<int>state2={StateSet::PRESSED,StateSet::FOCUSED,StateSet::HOVERED};
    std::vector<int>colors={0xFF0000,0x0000FF};
    std::vector<std::vector<int>>states;
    states.push_back(state1);
    states.push_back(state2);
    ASSERT_TRUE(StateSet::stateSetMatches(states[0],state1));
    ASSERT_TRUE(StateSet::stateSetMatches(stateEmpty,state1));
}

TEST_F(COLORSTATESET,getColorForState){
    std::vector<int>state1={StateSet::PRESSED,StateSet::FOCUSED,-StateSet::HOVERED};
    std::vector<int>state2={StateSet::PRESSED,-StateSet::FOCUSED,StateSet::HOVERED};
    std::vector<int>state3={StateSet::PRESSED,StateSet::FOCUSED,StateSet::HOVERED};
    std::vector<int>state4={-StateSet::PRESSED,StateSet::FOCUSED,StateSet::HOVERED};

    std::vector<int>match1={StateSet::PRESSED,StateSet::HOVERED};
    std::vector<int>match2={StateSet::PRESSED,StateSet::HOVERED,StateSet::FOCUSED};
    std::vector<int>match3={StateSet::PRESSED,StateSet::FOCUSED};
    std::vector<int>match4={StateSet::PRESSED};
    std::vector<int>match5={StateSet::FOCUSED,StateSet::HOVERED};

    std::vector<int>colors={1111,2222,3333,4444};
    std::vector<std::vector<int>>states;
    std::vector<std::vector<int>>statesEmpty;
    states.push_back(state1);
    states.push_back(state2);
    states.push_back(state3);
    states.push_back(state4);

    ColorStateList cs(states,colors); 

    ASSERT_EQ(colors[0],cs.getColorForState(state1,0));
    ASSERT_EQ(colors[1],cs.getColorForState(state2,0));

    ASSERT_EQ(colors[1],cs.getColorForState(match1,0));//-FOCUSED don't caret,so matched item is colors[1]
    ASSERT_EQ(colors[2],cs.getColorForState(match2,0));
    ASSERT_EQ(colors[0],cs.getColorForState(match3,0));
    ASSERT_EQ(0,cs.getColorForState(match4,0));
    ASSERT_EQ(colors[3],cs.getColorForState(match5,0));

    ColorStateList cs0(statesEmpty,colors);
    ASSERT_EQ(0,cs0.getColorForState(match5,0));
}
TEST_F(COLORSTATESET,buttonstate){
    std::vector<int>state0={StateSet::PRESSED};//pressed
    std::vector<int>state1={StateSet::ENABLED,StateSet::FOCUSED};//focused enabled
    std::vector<int>state2={StateSet::ENABLED};//enabled
    std::vector<int>state3={StateSet::FOCUSED};//focused
    std::vector<int>state4={};//none
    std::vector<std::vector<int>>states={state0,state1,state2,state3,state4};

    std::vector<int>empty;
    std::vector<int>s0={StateSet::PRESSED};
    std::vector<int>s1={StateSet::ENABLED,StateSet::PRESSED};
    std::vector<int>s2={StateSet::FOCUSED,StateSet::ENABLED};
    std::vector<int>s3={StateSet::ENABLED};
    std::vector<int>s4={StateSet::FOCUSED};

    ASSERT_EQ(index(states,s0),0);
    ASSERT_EQ(index(states,s1),0);
    ASSERT_EQ(index(states,s2),1);
    ASSERT_EQ(index(states,s3),2);
    ASSERT_EQ(index(states,s4),3);
    ASSERT_EQ(index(states,empty),4);
}

TEST_F(COLORSTATESET,match2){
    std::vector<int>state0={/*-StateSet::ENABLED,*/ StateSet::FOCUSED,-StateSet::SELECTED,-StateSet::PRESSED,-StateSet::HOVERED};//focused
    std::vector<int>state1={/*-StateSet::ENABLED,*/-StateSet::FOCUSED,-StateSet::SELECTED,-StateSet::PRESSED,-StateSet::HOVERED};//allmatch
    std::vector<std::vector<int>>states={state0,state1};

    std::vector<int>match0={StateSet::FOCUSED};
    std::vector<int>match1={StateSet::ENABLED};
    std::vector<int>match2={StateSet::FOCUSED,StateSet::ENABLED};
    std::vector<int>match3={};
    ASSERT_TRUE(StateSet::stateSetMatches(state0,match0));
    ASSERT_TRUE(StateSet::stateSetMatches(state1,match1));

    ASSERT_EQ(index(states,match2),0);
    ASSERT_EQ(index(states,match3),-1);
}

