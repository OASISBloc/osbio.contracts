#include "tokenswap.hpp"

using namespace eosio;
using namespace eosio::internal_use_do_not_use;

void tokenswap::senddat(name account, uint64_t quantity) {
    require_auth(account);
    
}

void tokenswap::getdat(name account, name from, uint64_t quantity) {
    
    require_auth(account);
    swapdata_index swapdataTable(get_self(), get_first_receiver().value);
    auto iterator = swapdataTable.find(account.value);
    
    eosio_assert(iterator != swapdataTable.end(), "Account data is not exist");

    eosio_assert(quantity <= iterator->osbAmount, "OSB balance is insufficient to swap DAT token");

    eosio::asset claim_token(quantity * 10000, eosio::symbol("DAT",4));

    action(
       permission_level{from, "active"_n},
       TOKEN_CONTRACT, "transfer"_n,
       std::make_tuple(from, account, claim_token, std::string("DAT token sent"))
    ).send();

    swapdataTable.modify(iterator, _self, [&](auto& row) {
        row.osbAmount = iterator->osbAmount - quantity;
        row.datAmount = iterator->datAmount + quantity;
    });

    eosio::print("[getdat function] ", "Account: ", iterator->account, ", OSB Amount: ", iterator->osbAmount, ", DAT Amount: ", iterator->datAmount);
    
}

void tokenswap::sendosb(name account, name to, uint64_t quantity) {

    require_auth(account);

    eosio::asset claim_token(quantity * 10000, eosio::symbol("OSB",4));

    action(
       permission_level{account, "active"_n},
       TOKEN_CONTRACT, "transfer"_n,
       std::make_tuple(account, to, claim_token, std::string("OSB token sent to tokenswap.io"))
    ).send();

    swapdata_index swapdataTable(get_self(), get_first_receiver().value);
    auto iterator = swapdataTable.find(account.value);
    if(iterator == swapdataTable.end()) {
        swapdataTable.emplace(_self, [&](auto& row) {
            row.account = to/*account*/;
            row.osbAmount = quantity;
            row.datAmount = 0;
            
            eosio::print("[sendosb function emplace] ", "Account: ", row.account, ", OSB Amount: ", row.osbAmount, ", DAT Amount: ", row.datAmount);
        });    
    } else {
        swapdataTable.modify(iterator, _self, [&](auto& row) {
            row.osbAmount += quantity;    

            eosio::print("[sendosb function modify] ", "Account: ", row.account, ", OSB Amount: ", row.osbAmount, ", DAT Amount: ", row.datAmount);
        });              
    }

}

void tokenswap::getosb(name account, uint64_t quantity) {
    require_auth(account);


}
 
void tokenswap::erase(name account) {
    require_auth(account);
    swapdata_index swapdataTable(get_self(), get_first_receiver().value);
    auto iterator = swapdataTable.find(account.value);
    check(iterator != swapdataTable.end(), "Record does not exist");
    swapdataTable.erase(iterator);
}

void tokenswap::test(name account, uint64_t quantity) {
    
    require_auth(account);

    swapdata_index swapdataTable(get_self(), get_first_receiver().value);
    auto iterator = swapdataTable.find(account.value);
    if(iterator == swapdataTable.end()) {
        swapdataTable.emplace(_self, [&](auto& row) {
            row.account = account;
            row.osbAmount = quantity;
            row.datAmount = 0;
            
            eosio::print("[TEST - sendosb function emplace] ", "Account: ", row.account, ", OSB Amount: ", row.osbAmount, ", DAT Amount: ", row.datAmount);
        });    
    } else {
        swapdataTable.modify(iterator, _self, [&](auto& row) {
            row.osbAmount += quantity;    

            eosio::print("[TEST - sendosb function modify] ", "Account: ", row.account, ", OSB Amount: ", row.osbAmount, ", DAT Amount: ", row.datAmount);
        });              
    }

}
    

EOSIO_DISPATCH( tokenswap, (senddat)(getdat)(sendosb)(getosb)(erase)(test) )
