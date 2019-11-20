

#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosio/print.hpp>


namespace eosiosystem {

   using eosio::current_time_point;
   using eosio::microseconds;
   using eosio::token;


   //hdkim(OASISBloc)
   void system_contract::startepoa( const name& owner ) {
      //require_auth( owner );
      //check( top_producers.size()  > 0 , "OasisBloc must have get at least one BG!" );
      block_timestamp timestamp;
      /*_gstate*/_gstate4.last_producer_schedule_update_time = 0; // By wschoi(OASISBloc)
      push_schedule(timestamp);
   }

   void system_contract::onblock( ignore<block_header> ) {
      using namespace eosio;

      require_auth(get_self());

      block_timestamp timestamp;
      name producer;
      _ds >> timestamp >> producer;

      // _gstate2.last_block_num is not used anywhere in the system contract code anymore.
      // Although this field is deprecated, we will continue updating it for now until the last_block_num field
      // is eventually completely removed, at which point this line can be removed.
      _gstate2.last_block_num = timestamp;

      /** until activated stake crosses this threshold no new rewards are paid */

  //    if( _gstate.total_activated_stake < min_activated_stake )
  //       return;


      if( _gstate.last_pervote_bucket_fill == time_point() )  /// start the presses
         _gstate.last_pervote_bucket_fill = current_time_point();


      /**
       * At startup the initial producer may not be one that is registered / elected
       * and therefore there may be no producer object for them.
       */
      auto prod = _producers.find( producer.value );
      if ( prod != _producers.end() ) {

        //hdkim(OASISBloc)
        // check( prod->stake  > min_stake_per_BG , "The BG must deposit at least 500,000 OSB" );

         //블록 생산중인 BG에 한번씩은 해야 할 로직
         if(prod->owner != /*_gstate*/_gstate4.now_work_BG ){ // By wschoi(OASISBloc)
            /*_gstate*/_gstate4.now_work_BG = prod->owner;

            //이전 BG의 규정 준수도 중 블록 생성 관련 스코어링
            uint8_t blocks_gas_penalty = 0;
            uint8_t once_bg_in_succession = 1;
            if(/*_gstate*/_gstate4.count_generated_block / blocks_per_once_round  > 1)
                once_bg_in_succession = /*_gstate*/_gstate4.count_generated_block / blocks_per_once_round ;

            if(/*_gstate*/_gstate4.count_generated_block % blocks_per_once_round != 0)
                blocks_gas_penalty = -5 * once_bg_in_succession;
            else
                blocks_gas_penalty = 1 * once_bg_in_succession;



            // by wschoi(OASISBloc)
            auto prod2 = _producers3.find( producer.value );
            if(prod2 != _producers3.end()) {
                _producers3.modify( prod2, same_payer, [&](auto& p ) {
                    p.compliance += blocks_gas_penalty;
                });
            }
            //_producers.modify( prod , same_payer, [&](auto& p ) { p.compliance += blocks_gas_penalty; });





            //마지막 BG가 12개의 블록을 다 생성하지 못하면 새로운 스케줄이 들어 가지 않는다.
            //따라서, 이 부분에서 새로운 스케줄을 넣어 준다.
            auto it  = _ePoA_state.begin();
            if(it->last_bg_woking_state == true){
                 _ePoA_state.modify( it, same_payer, [&]( auto& v ) {v.last_bg_woking_state = false; });
                 push_schedule(timestamp);
            }
            /*_gstate*/_gstate4.count_generated_block = 1;

         } else {
            /*_gstate*/_gstate4.count_generated_block ++;

            //마지막 BG의 마지막 블록 생성 시 새로운 스케줄을 push한다.
            auto it  = _ePoA_state.begin();
            if( prod->owner  == it->ending_bg_name_schedule && /*_gstate*/_gstate4.count_generated_block % blocks_per_once_round == 0 ){
               _ePoA_state.modify( it, same_payer, [&]( auto& v ) { v.last_bg_woking_state = false;});
               push_schedule(timestamp);
            }
            else if( prod->owner == it->ending_bg_name_schedule && /*_gstate*/_gstate4.count_generated_block % blocks_per_once_round != 0 ){
               _ePoA_state.modify( it, same_payer, [&]( auto& v ) { v.last_bg_woking_state = true;});
            }
         }


         //해당 BG가 생산한 블록 업 카운팅
         _gstate.total_unpaid_blocks++;
         _producers.modify( prod, same_payer, [&](auto& p ) {
               p.unpaid_blocks++;
         });
      }

/*
      /// only update block producers once every minute, block_timestamp is in half seconds
      if( timestamp.slot - _gstate.last_producer_schedule_update.slot > 120 ) {
         update_elected_producers( timestamp );

         if( (timestamp.slot - _gstate.last_name_close.slot) > blocks_per_day ) {
            name_bid_table bids(get_self(), get_self().value);
            auto idx = bids.get_index<"highbid"_n>();
            auto highest = idx.lower_bound( std::numeric_limits<uint64_t>::max()/2 );
            if( highest != idx.end() &&
                highest->high_bid > 0 &&
                (current_time_point() - highest->last_bid_time) > microseconds(useconds_per_day) &&
                _gstate.thresh_activated_stake_time > time_point() &&
                (current_time_point() - _gstate.thresh_activated_stake_time) > microseconds(14 * useconds_per_day)
            ) {
               _gstate.last_name_close = timestamp;
               channel_namebid_to_rex( highest->high_bid );
               idx.modify( highest, same_payer, [&]( auto& b ){
                  b.high_bid = -b.high_bid;
               });
            }
         }
      }
      */


   }

   void system_contract::claimrewards( const name& owner ) {
      require_auth( owner );
      eosio::print( "Clairm owner ", name{owner},"  " );

         //hdkim(OASISBloc)
      const int32_t PERIOD1 = 5;  const int32_t PERIOD2 = 8; const int32_t PERIOD3 = 10;
      const uint32_t one_year                =  365;
      const uint32_t deploy_day              =  1;  //리워드 간격-day
      const uint32_t reward_period_per_year  =  one_year/deploy_day;
      const uint64_t period_token_supply_0_5   = 200000000;
      const uint64_t period_token_supply_6_13  = 50000000;
      const uint64_t period_token_supply_14_23 = 10000000;
      const int32_t precision = 10000;
      const int32_t bg_count = 4;
      uint32_t reward_divied_bg = 10;
      const int64_t base_time = 1534291200; /// about 2019-08-26

      const auto& prod = _producers.get( owner.value );
      check( prod.active(), "producer does not have an active key" );
/*
      check( _gstate.total_activated_stake >= min_activated_stake,
                    "cannot claim rewards until the chain is activated (at least 15% of all tokens participate in voting)" );
                    */

      const auto ct = current_time_point();

      //BG 각 노드는 최소 24시간이 지나야 크레임을 할 수 있다.
      //check( ct - prod.last_claim_time > microseconds(useconds_per_day), "already claimed rewards within past day" );

      //current_block_time
      int64_t dur_time_sec = double(current_time_point().sec_since_epoch() - base_time);

      //eosio::print( "dur_time_sec ", dur_time_sec,"      " );
      //eosio::print( "base_time ", base_time, "      " );

      //check( ct - prod.last_claim_time > microseconds(useconds_per_day), "already claimed rewards within past day" );

     // uint64_t tbn = eosio::tapos_block_num();

      const asset token_supply   = token::get_supply(token_account, core_symbol().code() );
      eosio::internal_use_do_not_use::eosio_assert (0 < double(token_supply.amount), "check token issue");

      const auto usecs_since_last_fill = (ct - _gstate.last_pervote_bucket_fill).count();
      //eosio::print("time gap: ", usecs_since_last_fill / 1000'000ll,"sec   ");

      if( usecs_since_last_fill > 0 && _gstate.last_pervote_bucket_fill > time_point() ) {
         //auto new_tokens = static_cast<int64_t>( (continuous_rate * double(token_supply.amount) * double(usecs_since_last_fill)) / double(useconds_per_year) );
         double new_tokens = 0. ;

         if( dur_time_sec <= PERIOD1 * seconds_per_year ){  //블록 넘버가 5년 이하일 경우
            //BG 한 노드가 하루에 발행할 토큰 양
            new_tokens = static_cast<double>( period_token_supply_0_5 / reward_period_per_year );
            //eosio::print("Issue per BG: ", new_tokens,"  ");

            new_tokens  =  (new_tokens * usecs_since_last_fill / 1000'000ll) / seconds_per_day ;
            //eosio::print("time gap, Issue per BG: ", new_tokens,"  ");
            reward_divied_bg = 10; //bg 10%
            //eosio::print("The first priod - issue tokens: ", new_tokens,"  ");
         }
         else if( ( dur_time_sec > PERIOD1 * seconds_per_year   )  &&  ( dur_time_sec <= PERIOD2 * seconds_per_year ) ) {  //블록 넘버가 6~13년 미만
            //BG 한 노드가 하루에 발행할 토큰 양
            new_tokens = static_cast<double>( period_token_supply_6_13 / reward_period_per_year );
            new_tokens  =  (new_tokens * usecs_since_last_fill / 1000'000ll ) / seconds_per_day;
            reward_divied_bg = 20; //bg 20%
         }
         else if( ( dur_time_sec > PERIOD2 * seconds_per_year )  &&  ( dur_time_sec <= PERIOD3 * seconds_per_year  ) ) {  //블록 넘버가 13~23년 미만
         //BG 한 노드가 하루에 발행할 토큰 양
            new_tokens = static_cast<double>( period_token_supply_14_23 / reward_period_per_year);
            new_tokens  =  (new_tokens * usecs_since_last_fill  / 1000'000ll) / seconds_per_day;
            reward_divied_bg = 30;//bg 30%
         } else{
            eosio::print("           error token reward");
         }
         eosio::internal_use_do_not_use::eosio_assert( new_tokens > 0 , "check new_tokens" );

         auto to_producers     = new_tokens;
         auto to_savings       = to_producers * (100 - reward_divied_bg)  / 100 ;
         auto to_per_block_pay = to_producers  * reward_divied_bg  / 100 ;

         auto to_per_vote_pay  = 0;  //hdkim 수정할 것..
         {
            token::issue_action issue_act{ token_account, { {get_self(), active_permission} } };
            issue_act.send( get_self(), asset(to_producers * precision, core_symbol()), "issue tokens for producer pay and savings" );
         }
         {
            token::transfer_action transfer_act{ token_account, { {get_self(), active_permission} } };
            transfer_act.send( get_self(), saving_account, asset(to_savings * precision, core_symbol()), "unallocated inflation" );
            transfer_act.send( get_self(), bpay_account,   asset(to_per_block_pay * precision , core_symbol()), "fund per-block bucket" );
            //PoA -  투표에 의한 리워드는 없음.
         //   transfer_act.send( get_self(), vpay_account, asset(to_per_vote_pay, core_symbol()), "fund per-vote bucket" );
         }

         _gstate.pervote_bucket          += to_per_vote_pay;
         _gstate.perblock_bucket         += to_per_block_pay;
         _gstate.last_pervote_bucket_fill = ct;
      }

      auto prod2 = _producers2.find( owner.value );

      /// New metric to be used in pervote pay calculation. Instead of vote weight ratio, we combine vote weight and
      /// time duration the vote weight has been held into one metric.

      const auto last_claim_plus_3days = prod.last_claim_time + microseconds(3 * useconds_per_day);

      bool crossed_threshold       = (last_claim_plus_3days <= ct);
      bool updated_after_threshold = true;
      if ( prod2 != _producers2.end() ) {
         updated_after_threshold = (last_claim_plus_3days <= prod2->last_votepay_share_update);
      } else {
         prod2 = _producers2.emplace( owner, [&]( producer_info2& info  ) {
            info.owner                     = owner;
            info.last_votepay_share_update = ct;
         });
      }


      // Note: updated_after_threshold implies cross_threshold (except if claiming rewards when the producers2 table row did not exist).
      // The exception leads to updated_after_threshold to be treated as true regardless of whether the threshold was crossed.
      // This is okay because in this case the producer will not get paid anything either way.
      // In fact it is desired behavior because the producers votes need to be counted in the global total_producer_votepay_share for the first time.

      eosio::print("total_unpaid_blocks: ",_gstate.total_unpaid_blocks,"  ");
      eosio::print("prod.unpaid_blocks: " ,prod.unpaid_blocks,"   ");
      eosio::print("perblock_bucket: "    ,_gstate.perblock_bucket,"    ");

      //미지급된 전체 total_unpaid_blocks 중에서 내가 받아야 할 unpaid_blocks 계산
      double producer_per_block_pay = 0.;
      if( _gstate.total_unpaid_blocks > 0 ) {
         producer_per_block_pay = static_cast<double>( (_gstate.perblock_bucket * prod.unpaid_blocks) / _gstate.total_unpaid_blocks ) ;
      }

       eosio::print("producer_per_block_pay: "    ,producer_per_block_pay,"    ");


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //아래 코드는 투표와 관련된 코드..지워도 될것 같은데 혹시 몰라 남겨둠.. 코드가 남아 있어도 오아시스 블록에 별 지장 안줄 듯..
      double new_votepay_share = update_producer_votepay_share( prod2,
                                    ct,
                                    updated_after_threshold ? 0.0 : prod.total_votes,
                                    true // reset votepay_share to zero after updating
                                 );

      int64_t producer_per_vote_pay = 0;
      if( _gstate2.revision > 0 ) {
         double total_votepay_share = update_total_votepay_share( ct );
         if( total_votepay_share > 0 && !crossed_threshold ) {
            producer_per_vote_pay = int64_t((new_votepay_share * _gstate.pervote_bucket) / total_votepay_share);
            if( producer_per_vote_pay > _gstate.pervote_bucket )
               producer_per_vote_pay = _gstate.pervote_bucket;
         }
      } else {
         if( _gstate.total_producer_vote_weight > 0 ) {
            producer_per_vote_pay = int64_t((_gstate.pervote_bucket * prod.total_votes) / _gstate.total_producer_vote_weight);
         }
      }

      if( producer_per_vote_pay < min_pervote_daily_pay ) {
         producer_per_vote_pay = 0;
      }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

       //버켓이나 토탈 언페이드 블록에서 내가 일한 만큼 차감함.
      _gstate.pervote_bucket      -= producer_per_vote_pay;
      _gstate.perblock_bucket     -= producer_per_block_pay;
      _gstate.total_unpaid_blocks -= prod.unpaid_blocks;

      //아래 코드는 투표와 관련된 코드임..별 영향 안줄 듯..
      update_total_votepay_share( ct, -new_votepay_share, (updated_after_threshold ? prod.total_votes : 0.0) );

      _producers.modify( prod, same_payer, [&](auto& p) {
         p.last_claim_time = ct;
         p.unpaid_blocks   = 0;
      });


      //BG가 크레임을 할 때 각 BG가 생성산 만큼 토큰이 전송되는 코드.
      if ( producer_per_block_pay > 0. ) {
         eosio::print("producer_per_block_pay  ",producer_per_block_pay,"    ");
         token::transfer_action transfer_act{ token_account, { {bpay_account, active_permission}, {owner, active_permission} } };
         transfer_act.send( bpay_account, owner, asset(producer_per_block_pay * precision , core_symbol()), "producer block pay" );
      }

      /*
      if ( producer_per_vote_pay > 0 ) {
         eosio::print("producer_per_vote_pay > 0    ",producer_per_vote_pay,"    ");
         token::transfer_action transfer_act{ token_account, { {vpay_account, active_permission}, {owner, active_permission} } };
         transfer_act.send( vpay_account, owner, asset(producer_per_vote_pay, core_symbol()), "producer vote pay" );
      }
      */
   }


} //namespace eosiosystem
