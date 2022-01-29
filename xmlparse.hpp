/*
 * Description: interfaces to parse XML data
 * Author: Laura Galbraith
*/

#ifndef SVGSC_XMLPARSE_H
#define SVGSC_XMLPARSE_H

#include <string> // string
#include <vector> // vector

class XMLParseResult {
  public:
    // Constructor
    XMLParseResult(const std::vector<std::string>& result_data, const std::string& result_error);

    // Member variables
    std::vector<std::string> data;
    std::string error; // will be empty if result is successful
};

// returns all desired html page data, preceded by the specified element containing the specified substring, and nested within the specified elements
XMLParseResult GetPrecededAndNestedData(
  const std::string& page_data,
  const std::string& preceding_element_name,
  const std::string& preceding_element_data_substring,
  const std::vector<std::string>& containing_element_names);

#endif // SVGSC_XMLPARSE_H
