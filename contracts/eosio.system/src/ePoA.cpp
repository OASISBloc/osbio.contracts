// Copyright (c) 2018-2019 OASIS LTD.
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <eosio.system/eosio.system.hpp>
#include <eosio/eosio.hpp>
#include <eosio/transaction.hpp>


namespace eosiosystem {      
   void system_contract::push_schedule(const block_timestamp& block_time){
      using namespace eosio;           

     // By wschoi(OASISBloc)
     //eosio::print("_gstate.last_producer_schedule_size: ",_gstate.last_producer_schedule_size,"  ");     
      if( current_time_point().sec_since_epoch() - /*_gstate*/_gstate4.last_producer_schedule_update_time > /*_gstate*/_gstate4.last_bg_total * blocks_per_once_round * 2  ) {
         eosio::print("_gstate4.last_producer_schedule_update_time : ",/*_gstate*/_gstate4.last_producer_schedule_update_time ,"  ");
         eosio::print("current_time_point().sec_since_epoch() : ",current_time_point().sec_since_epoch() ,"  ");
         /*_gstate*/_gstate4.last_producer_schedule_update_time = current_time_point().sec_since_epoch();
      }
      else return;
      
      auto idx = _producers.get_index<"prototalvote"_n>();
      std::vector< std::pair<double,eosio::producer_key> > top_producers;
      top_producers.reserve(WG_total);                    

      convert_amount_of_stake_quantity_to_table();
      //WG의 포인트 계산 후 리스트 추가
       
      for ( auto it = idx.cbegin(); it != idx.cend() ; ++it ) {         
         if(it->active()){
            double bg_point = calculate_level_point_by_witness_group(it->owner);         
            top_producers.emplace_back(std::pair<double,eosio::producer_key>({bg_point, {it->owner, it->producer_key}}));         
         }        
      }

      eosio::print("WG count: ",top_producers.size()," ");

      if ( top_producers.size() < WG_min_point ) { 
         check( top_producers.size() >= WG_min_point , "OasisBloc must have get at least 8 BG!" );
         return;  
      }

      //포인트로 WG oder 줄을 세운다.  
      std::sort(top_producers.begin(), top_producers.end(), std::greater<std::pair<double,eosio::producer_key>>() );   
      
      //rand
      auto ct        = current_time_point().sec_since_epoch(); 
      auto seedValue = tapos_block_prefix() * tapos_block_num();
      osb_srand((uint32_t) (seedValue * ct));            

      //레벨 1 -  3노드 중 1개 제외  - 1~3 대상
      //레벨 2 -  4노드 중 2개 제외  - 4~7 대상
      //레벨 2 -  6노드 중 3개 제외  - 7~13 대상
      //레벨별 제외할 BG의 번호를 난수를 통해서 선정     
      
      int be_deactived_bg = static_cast<int>((WG_total - top_producers.size()) * 0.6 );  
      eosio::print("(WG_total - top_producers.size()): ", (WG_total - top_producers.size()) ,"   ");
      eosio::print("be_deactived_bg: ", be_deactived_bg," ");

      std::vector<int> exclude_bg;
      exclude_bg.reserve(WG_total-BG_total-be_deactived_bg);      
      exclude_bg.emplace_back(osb_rand() % 3);
      exclude_bg.emplace_back(3 + osb_rand() % 4);
      exclude_bg.emplace_back(3 + osb_rand() % 4);     
      for(int i = 0; i < 3 - be_deactived_bg; i++ ){
         exclude_bg.emplace_back(7 + osb_rand() % (6 - (WG_total - top_producers.size())));         
      }
      while(exclude_bg[1] == exclude_bg[2] ) exclude_bg[2] = 3 + osb_rand() % 4;     

      if(be_deactived_bg == 0){ //BG가 13 모두 active 일때 
         while (1) {         
            if(exclude_bg[3] != exclude_bg[5]) {
               if(exclude_bg[4] != exclude_bg[5]) break;
               else{
                  exclude_bg[5] = 7 + osb_rand() % 6;
                  continue;
               } 
            }else {
               exclude_bg[5] = 7 + osb_rand() % 6;
               continue;
            } 
         }         
      }else if(be_deactived_bg == 1)  { //BG가 10, 11 개 active 일때 
         while(exclude_bg[3] == exclude_bg[4] ) exclude_bg[4] = 7 + osb_rand() % 6 - (WG_total - top_producers.size());
      }

      std::sort( exclude_bg.begin() , exclude_bg.end() ); //제외 되어야 할 BG의 순서 sort    

      for(int i=0;i<exclude_bg.size();i++){
         eosio::print("exclude_bg ",i,": ", exclude_bg[i]," ");
      }

      //레벨화를 통해 BG 선정 
      int iner_cnt = 0;  int cnt=0;                 
      std::vector<eosio::producer_key> producers;          
      producers.reserve(BG_total);           
      for( const auto& item : top_producers ){                           
         if(exclude_bg[cnt] != iner_cnt)  producers.push_back(item.second);
         else cnt ++; 
         iner_cnt++;            
      }           

      // By wschoi(OASISBloc)
      /*_gstate*/_gstate4.last_bg_total = producers.size();
      
      //상태 테이블에 필요 정보 입력
      if(producers.size() > 1 ){
         auto bg_begin = producers.begin();
         auto bg_end   = producers.end() - 1;
         auto vitr     = _ePoA_state.begin();

         if(_ePoA_state.begin() == _ePoA_state.end()  ){  //first input data
               _ePoA_state.emplace( get_self(), [&]( auto& v ) {               
               v.starting_bg_name_schedule = bg_begin->producer_name;
               v.ending_bg_name_schedule   = bg_end->producer_name;
               v.total_BG                  = producers.size();
             });
            
         } else {
            _ePoA_state.modify( vitr, same_payer, [&]( auto& v ) {               
               v.starting_bg_name_schedule = bg_begin->producer_name;
               v.ending_bg_name_schedule   = bg_end->producer_name;               
               v.total_BG                  =  producers.size();
            });
         }
      }       

      if( set_proposed_producers( producers ) >= 0 ) {
         _gstate.last_producer_schedule_size = static_cast<decltype(_gstate.last_producer_schedule_size)>( top_producers.size() );
      } 
      
   }  //end of system_contract::push_schedule    


   void system_contract::convert_amount_of_stake_quantity_to_table(){
      for(auto& BG : _producers) { 
         //user resource에서 cpu, net 확인
         user_resources_table bg_tbl( get_self(), BG.owner.value );      
         auto itr  = bg_tbl.find( BG.owner.value );
         if ( itr != bg_tbl.end() ) {
            if( min_stake_per_BG <= itr->net_weight.amount + itr->cpu_weight.amount ){

               // By wschoi(OASISBloc)
               auto prod  = /*_producers*/_producers3.find( BG.owner.value );
               if ( prod != /*_producers*/_producers3.end() ) {
                  /*_producers*/_producers3.modify( prod, same_payer, [&](auto& p ) {
                     p.stake         = itr->net_weight.amount + itr->cpu_weight.amount;                      
                     p.deposit_point = (itr->net_weight.amount + itr->cpu_weight.amount) / 10000 - min_stake_per_BG;                     
                  });                     
               }

            }
         } //end of if
      } // end of for
   }

   double system_contract::calculate_level_point_by_witness_group(const name bg){

      //auto prod = _producers.find( bg.value );
      auto prod = _producers3.find( bg.value );  // by wschoi(OASISBloc)

      if ( prod != /*_producers*/_producers3.end() ) {
         return prod->deposit_point * 0.4 + prod->compliance * 0.3 + prod->contribution * 0.3;
      }else{
         return 0.0;
      }
   }
}