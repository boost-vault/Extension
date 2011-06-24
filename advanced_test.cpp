//  (C) Copyright Jeremy Pack 2006.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#include <boost/thread.hpp>
#include <boost/extension/extension.hpp>
#include <boost/extension/repository.hpp>
#include <boost/extension/loadable.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test_framework.hpp>
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

using namespace boost::extensions;
/*The classes below shows the most powerful way of declaring
loadable classes. For this method, each class must have 
extension in it's inheritance hierarchy.*/

/*
structure is not itself loadable, but classes that inherit
from it are loadable as structures
*/
class structure : public extension
{
public:
  /*
    repository contains any shared variables and interfaces
    that are relevant for multiple loadable classes.
  */
  structure(repository & rep):extension(rep){}
  //This function is just here for demonstration.
  virtual std::string get_my_name(){return "generic structure";}
  /*
    Virtual destructors are not required - the extension
    virtual destructor takes care of those issues - but
    without virtual destructors, some compilers will give
    warnings (when a class has other virtual functions)
  */
  virtual ~structure(){}

};
/*
Notice that this class below is also not exported - though
it could be.
*/
class garage : public structure
{
protected:
  static void get_interface_info(boost::extensions::library_class & lc)
  {
    /*
    This function declares that
  garage is loadable as a structure, or as a garage.
  Dependencies could also be declared here:
  for example: lc.require<car>();
*/
    lc.provide<garage>();
    lc.provide<structure>();
  }
public:
  /*
    This style of constructor (with a single repository argument)
    is the simplest to use -
    but different constructors can be created with some work.
    They are not necessary, since all parameters can be placed
    in the repository, but in some cases a more specialized
    constructor is needed, and the library does not restrict this.
  */
  garage(repository & rep):structure(rep){}
  virtual std::string get_my_name(){return "some sort of garage";}
  virtual ~garage(){}
};
/*
house is exported. Notice the functions:

generate
get_extension_info - provide basic description
get_interface_info - list requirements and provisions

  The last two are separate so that a class can recursively
  include the dependencies of its super classes, if desired.
*/
class house : public structure
{
private:
  static void get_interface_info(boost::extensions::library_class & lc)
  {
    /*To construct a house, a garage is required to have been
    constructed.
    */
    lc.provide<house>();
    lc.provide<structure>();
    lc.require<garage>();
  }
  garage * garage_ptr;//pointer to the required garage
public:
  std::string get_garage_name(){return garage_ptr->get_my_name();}
  virtual std::string get_my_name(){return "some sort of house";}
  virtual ~house(){}
  static boost::extensions::extension * generate(boost::extensions::repository & rep/*, boost::extensions::compatible_extensions & ext_ptrs*/)
  {return new house(rep);}//standard generate function - required
  static void get_extension_info(boost::extensions::library_class & lc)
  {//called when a class is declared loadable
    lc.describe("A basic house");
    get_interface_info(lc);
  }
  house(boost::extensions::repository & rep)
    :structure(rep)
  {
    rep.set_to_first(garage_ptr);//Load the first available garage
  }

};
//similar to house
class four_car_garage : public garage
{
protected:
  static void get_interface_info(boost::extensions::library_class & lc)
  {
    lc.provide<four_car_garage>();
  }
public:
  static extension * generate(repository & rep){return new four_car_garage(rep);}
  static void get_extension_info(boost::extensions::library_class & lc)
  {
    lc.describe("A four car garage");
    get_interface_info(lc);
    garage::get_interface_info(lc);
  }
  four_car_garage(repository & rep):garage(rep){}
  virtual std::string get_my_name(){return "a four car garage";}
};
//similar to house
class two_car_garage : public garage
{
protected:
  static void get_interface_info(boost::extensions::library_class & lc)
  {
    lc.provide<two_car_garage>();
  }
public:
  static extension * generate(repository & rep){return new two_car_garage(rep);}
   static void get_extension_info(boost::extensions::library_class & lc)
  {
    lc.describe("A 2 car garage");
    get_interface_info(lc);
    garage::get_interface_info(lc);
  }
  two_car_garage(repository & rep):garage(rep){}
  virtual std::string get_my_name(){return "a two car garage";}
};
BOOST_AUTO_UNIT_TEST(creation)
{
  /*Upon creation, the loader searches the current
  directory. It is possible to also manually specify other
  files or directories*/
  loader load;
  /*
  This next part is only necessary because the 
  classes are being loaded from the current executable.
                                */
  library * lib = new library();
  lib->make_class_available<two_car_garage>();
  lib->make_class_available<four_car_garage>();
  lib->make_class_available<house>();
  load.add_library("Local Classes", lib);//don't worry, library won't leak now
  //it is stored in a smart pointer.
  
  /*This single_loadable can carry exactly one instance of a garage
  it is initialized with the loader, and it points to a list
  of all available garages.
  */
  single_loadable<garage> garage_loader(load);
  //can load one house
  single_loadable<house> house_loader(load);
  /*No houses are available, becase the only house
  requires a garage, which has not been constructed.*/
  BOOST_CHECK_EQUAL(house_loader.get_num_available(), 0);
  /*Neither garage depends on anything, so they are both loadable*/
  BOOST_CHECK_EQUAL(garage_loader.get_num_available(), 2);
  if(garage_loader.get_num_available()!=2)
  {
    BOOST_CHECK(0);
    return;
  }
  /*Load the first garage*/
  garage_loader.load(garage_loader.get_class_begin());
  BOOST_CHECK_EQUAL(house_loader.get_num_available(), 1);
  /*Load the house - it will take a pointer to the 
  loaded garage from the repository
  */
  house_loader.load(house_loader.get_class_begin());
  /*make sure that the first garage has loaded successfully.*/
  BOOST_CHECK_EQUAL(std::string(house_loader->get_garage_name()), std::string("a two car garage"));
}