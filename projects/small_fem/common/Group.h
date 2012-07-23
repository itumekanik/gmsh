#ifndef _GROUP_H_
#define _GROUP_H_

#include <vector>
#include <string>

/**
   @interface Group
   @brief A group of elements

   This is the interface allowing @em partial @em access to a group
   of elements.@n

   Every Group has a particular type of element.@n
   Each type is represented by a number:
   @li 1 for Group of @em MElement%s
   @li 2 for Group of @em Dof%s

   For a @em full access, use implemented classes (GroupOf*).@n

   @warning
   Every class implementing this interface @em must also implement:
   @li A get() method to access a @em particular element of the Group
   @li A getAll() method to access @em all the elements of the Group

   These methods are @em not in this interface because of @em return @em type
   @em issues.

   @todo
   Find a solution for return type issuses, template and doxygen !!
 */

class Group{
 public:
  virtual ~Group(void);

  virtual int getNumber(void) const = 0;
  
  virtual int getId(void) const = 0; 

  virtual int getType(void) const = 0;

  virtual std::string toString(void) const = 0;
};


/**
   @fn Group::~Group
   Deletes this Group

   @fn Group::getNumber
   @return Returns the number of elements in the Group

   @fn Group::getId
   @return Returns the @c ID of this Group 

   @fn Group::getType
   @return Returns the type of the elements 

   @fn Group::toString
   @return Returns a string discribing this Group
*/


#endif