#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <math.h>
using namespace eosio;

class [[eosio::contract]] tokenswap : public contract {
    
public:
      
    tokenswap(name receiver, name code, datastream<const char*> ds ): contract(receiver, code, ds) {}

    const name TOKEN_CONTRACT = "osbio.token"_n;

    [[eosio::action]] void senddat(name account, uint64_t quantity);
      
    [[eosio::action]] void getdat(name account, name from, uint64_t quantity);
      
    [[eosio::action]] void sendosb(name account, name to, uint64_t quantity);

    [[eosio::action]] void getosb(name account, uint64_t quantity);

    [[eosio::action]] void erase(name account);
     

private:

    struct [[eosio::table]] swapdata {

        name account;

        uint64_t osbAmount;

        uint64_t datAmount;        

        uint64_t primary_key() const {
            return account.value;
        }    

    };

    typedef eosio::multi_index<"swapdata"_n, swapdata> swapdata_index;
    


};
