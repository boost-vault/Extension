//  (C) Copyright Jeremy Pack 2006.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test_framework.hpp>
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <boost/extension/config.hpp>
#include <boost/thread.hpp>
#include <boost/extension/loader.hpp>
#include <boost/extension/extension.hpp>
#include <boost/extension/intrusive_capable.hpp>
#include <boost/extension/loadable.hpp>
#include "number_base.hpp"

using namespace boost::extensions;
class car//this is an abstract base class - it doesn't have to be abstract though
{
public:
  virtual ~car(){}
  virtual const char * get_type()=0;
};
class chevy : public car
{
public:
  virtual const char * get_type(){return "Chevrolet";}
};
class honda : public car
{
public:
  virtual const char * get_type(){return "Honda";}
};
BOOST_AUTO_UNIT_TEST(basic)
{
  /*create loader*/
  loader load;
  /*make local classes loadable*/
  library * lib = new library();
  lib->make_non_extension_class_available<chevy, car>();
  lib->make_non_extension_class_available<honda, car>();
  load.add_library("Local Classes", lib);
  /*create an object that can hold multiple cars*/
  multi_loadable<car> car_ptr(load);
  /*Get an iterator to the first available loadable class*/
  available_class_iterator it = car_ptr.get_class_begin();
  /*Make sure the list isn't empty*/
  if(it==car_ptr.get_class_end())
  {
    BOOST_CHECK(0);//This shouldn't happen
    return;
  }
  /*Load the class pointed to by the iterator*/
  car_ptr.load(it);
  /*Check that the virtual function output is correct*/
  BOOST_CHECK_EQUAL(std::string("Chevrolet"), std::string(car_ptr[0].get_type()));
  /*Go to the next available loadable class*/
  ++it;
  if(it==car_ptr.get_class_end())
  {
    BOOST_CHECK(0);//This shouldn't happen
    return;
  }
  /*Load Honda*/
  car_ptr.load(it);
  BOOST_CHECK_EQUAL(std::string("Honda"), std::string(car_ptr[1].get_type()));
  /*There are now two cars loaded into car_ptr*/
}
