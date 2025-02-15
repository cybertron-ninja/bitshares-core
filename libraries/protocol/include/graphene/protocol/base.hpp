/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <graphene/protocol/ext.hpp>
#include <graphene/protocol/types.hpp>

#include <fc/thread/future.hpp>

namespace graphene { namespace protocol {
   struct asset;
   struct authority;

   /**
    *  @defgroup operations Operations
    *  @ingroup transactions Transactions
    *  @brief A set of valid comands for mutating the globally shared state.
    *
    *  An operation can be thought of like a function that will modify the global
    *  shared state of the blockchain.  The members of each struct are like function
    *  arguments and each operation can potentially generate a return value.
    *
    *  Operations can be grouped into transactions (@ref transaction) to ensure that they occur
    *  in a particular order and that all operations apply successfully or
    *  no operations apply.
    *
    *  Each operation is a fully defined state transition and can exist in a transaction on its own.
    *
    *  @section operation_design_principles Design Principles
    *
    *  Operations have been carefully designed to include all of the information necessary to
    *  interpret them outside the context of the blockchain.   This means that information about
    *  current chain state is included in the operation even though it could be inferred from
    *  a subset of the data.   This makes the expected outcome of each operation well defined and
    *  easily understood without access to chain state.
    *
    *  @subsection balance_calculation Balance Calculation Principle
    *
    *    We have stipulated that the current account balance may be entirely calculated from
    *    just the subset of operations that are relevant to that account.  There should be
    *    no need to process the entire blockchain inorder to know your account's balance.
    *
    *  @subsection fee_calculation Explicit Fee Principle
    *
    *    Blockchain fees can change from time to time and it is important that a signed
    *    transaction explicitly agree to the fees it will be paying.  This aids with account
    *    balance updates and ensures that the sender agreed to the fee prior to making the
    *    transaction.
    *
    *  @subsection defined_authority Explicit Authority
    *
    *    Each operation shall contain enough information to know which accounts must authorize
    *    the operation.  This principle enables authority verification to occur in a centralized,
    *    optimized, and parallel manner.
    *
    *  @subsection relevancy_principle Explicit Relevant Accounts
    *
    *    Each operation contains enough information to enumerate all accounts for which the
    *    operation should apear in its account history.  This principle enables us to easily
    *    define and enforce the @ref balance_calculation. This is superset of the @ref defined_authority
    *
    *  @{
    */

   struct void_result{};

   struct generic_operation_result
   {
      flat_set<object_id_type> new_objects;
      flat_set<object_id_type> updated_objects;
      flat_set<object_id_type> removed_objects;
   };

   struct generic_exchange_operation_result
   {
      vector<asset> paid;
      vector<asset> received;
      vector<asset> fees;
   };

   struct extendable_operation_result_dtl
   {
      optional<flat_set<account_id_type>> impacted_accounts;
      optional<flat_set<object_id_type>>  new_objects;
      optional<flat_set<object_id_type>>  updated_objects;
      optional<flat_set<object_id_type>>  removed_objects;
      optional<vector<asset>>             paid;
      optional<vector<asset>>             received;
      optional<vector<asset>>             fees;
   };

   using extendable_operation_result = extension<extendable_operation_result_dtl>;

   using operation_result = fc::static_variant <
         /* 0 */ void_result,
         /* 1 */ object_id_type,
         /* 2 */ asset,
         /* 3 */ generic_operation_result,
         /* 4 */ generic_exchange_operation_result,
         /* 5 */ extendable_operation_result
         >;

   struct base_operation
   {
      template<typename T>
      share_type calculate_fee(const T& params)const
      {
         return params.fee;
      }
      virtual void get_required_authorities( vector<authority>& )const{ /* do nothing by default */ }
      virtual void get_required_active_authorities( flat_set<account_id_type>& )const{ /* do nothing by default */ }
      virtual void get_required_owner_authorities( flat_set<account_id_type>& )const{ /* do nothing by default */ }
      virtual void validate()const{ /* do nothing by default */ }
      fc::optional< fc::future<void> > validate_parallel( uint32_t skip )const;

      static uint64_t calculate_data_fee( uint64_t bytes, uint64_t price_per_kbyte );
   };

   /**
    *  For future expansion many structus include a single member of type
    *  extensions_type that can be changed when updating a protocol.  You can
    *  always add new types to a static_variant without breaking backward
    *  compatibility.   
    */
   using future_extensions = static_variant<void_t>;

   /**
    *  A flat_set is used to make sure that only one extension of
    *  each type is added and that they are added in order.  
    *  
    *  @note static_variant compares only the type tag and not the 
    *  content.
    */
   using extensions_type = future_extensions::flat_set_type;

   ///@}

} } // graphene::protocol

FC_REFLECT_TYPENAME( graphene::protocol::extendable_operation_result )
FC_REFLECT_TYPENAME( graphene::protocol::operation_result )
FC_REFLECT_TYPENAME( graphene::protocol::future_extensions )
FC_REFLECT_TYPENAME( graphene::protocol::extensions_type )
FC_REFLECT( graphene::protocol::void_result, )
FC_REFLECT( graphene::protocol::generic_operation_result, (new_objects)(updated_objects)(removed_objects) )
FC_REFLECT( graphene::protocol::generic_exchange_operation_result, (paid)(received)(fees) )
FC_REFLECT( graphene::protocol::extendable_operation_result_dtl,
            (impacted_accounts)(new_objects)(updated_objects)(removed_objects)(paid)(received)(fees) )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::generic_operation_result ) // impl in operations.cpp
// impl in operations.cpp
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::generic_exchange_operation_result )
// impl in operations.cpp
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::extendable_operation_result_dtl )
