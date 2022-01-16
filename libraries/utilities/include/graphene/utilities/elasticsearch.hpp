/*
 * Copyright (c) 2018 oxarbitrage, and contributors.
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
#include <cstddef>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <fc/time.hpp>
#include <fc/variant_object.hpp>

namespace graphene { namespace utilities {

class curl_wrapper
{
public:
   curl_wrapper();

   // Note: the numbers are used in the request() function. If we need to update or add, please check the function
   enum class http_request_method
   {
      HTTP_GET     = 0,
      HTTP_POST    = 1,
      HTTP_HEAD    = 2,
      HTTP_PUT     = 3,
      HTTP_DELETE  = 4,
      HTTP_PATCH   = 5,
      HTTP_OPTIONS = 6
   };

   struct http_response_code
   {
      static constexpr uint16_t HTTP_200 = 200;
      static constexpr uint16_t HTTP_401 = 401;
      static constexpr uint16_t HTTP_413 = 413;
   };

   struct http_response
   {
      uint16_t    code;
      std::string content;
      bool is_200() const; ///< @return if @ref code is 200
   };

   http_response request( http_request_method method,
                          const std::string& url,
                          const std::string& auth,
                          const std::string& query ) const;

   http_response get( const std::string& url, const std::string& auth ) const;
   http_response del( const std::string& url, const std::string& auth ) const;
   http_response post( const std::string& url, const std::string& auth, const std::string& query ) const;
   http_response put( const std::string& url, const std::string& auth, const std::string& query ) const;

private:

   static CURL* init_curl();
   static curl_slist* init_request_headers();

   struct curl_deleter
   {
      void operator()( CURL* p_curl ) const;
   };

   struct curl_slist_deleter
   {
      void operator()( curl_slist* slist ) const;
   };

   std::unique_ptr<CURL, curl_deleter> curl { init_curl() };
   std::unique_ptr<curl_slist, curl_slist_deleter> request_headers { init_request_headers() };
};

class es_client
{
public:
   es_client( const std::string& p_base_url, const std::string& p_auth ) : base_url(p_base_url), auth(p_auth) {}

   bool check_status() const;
   std::string get_version() const;
   void check_version_7_or_above( bool& result ) const noexcept;

   bool send_bulk( const std::vector<std::string>& bulk_lines ) const;
   bool del( const std::string& path ) const;
   std::string get( const std::string& path ) const;
   std::string query( const std::string& path, const std::string& query ) const;
private:
   std::string base_url;
   std::string auth;
   curl_wrapper curl;
};

   class ES {
      public:
         CURL *curl;
         std::vector <std::string> bulk_lines;
         std::string elasticsearch_url;
         std::string index_prefix;
         std::string auth;
         std::string endpoint;
         std::string query;
   };
   class CurlRequest {
      public:
         CURL *handler;
         std::string url;
         std::string type;
         std::string auth;
         std::string query;
   };

   bool SendBulk(ES&& es);
   bool checkES(ES& es);
   std::string getESVersion(ES& es);
   void checkESVersion7OrAbove(ES& es, bool& result) noexcept;
   std::string simpleQuery(ES& es);
   bool deleteAll(ES& es);
   std::string getEndPoint(ES& es);

   std::string doCurl(CurlRequest& curl);

   std::vector<std::string> createBulk(const fc::mutable_variant_object& bulk_header, std::string&& data);

struct es_data_adaptor
{
   enum class data_type
   {
      static_variant_type,
      map_type,
      array_type // can be simple arrays, object arrays, static_variant arrays, or even nested arrays
   };

   static fc::variant adapt( const fc::variant_object& op );

   static fc::variant adapt( const fc::variants& v, data_type type );

   static fc::variant adapt_map_item( const fc::variants& v );

   static fc::variant adapt_static_variant( const fc::variants& v );

   /// In-place update
   static void adapt( fc::variants& v );

   /// Extract data from @p v into @p mv
   static void extract_data_from_variant( const fc::variant& v,
                                          fc::mutable_variant_object& mv,
                                          const std::string& prefix );

};

} } // end namespace graphene::utilities
