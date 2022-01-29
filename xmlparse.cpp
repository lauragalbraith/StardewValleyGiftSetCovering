/*
 * Description: implementations to parse XML data
 * Documentation/references:
 *    - libxml2 documentation: http://xmlsoft.org/html/libxml-HTMLparser.html
 *    - HTMLparser.h example with C++: https://curl.se/libcurl/c/htmltitle.html
 * Author: Laura Galbraith
*/

#include "xmlparse.hpp" // self-include header

#include <string> // string
#include <vector> // vector
#include <cctype> // tolower
#include <libxml/HTMLparser.h> // htmlSAXHandler, xmlChar, htmlParserCtxtPtr, htmlCreatePushParserCtxt, XML_CHAR_ENCODING_NONE, htmlParseChunk, htmlFreeParserCtxt

XMLParseResult::XMLParseResult(const std::vector<std::string>& result_data, const std::string& result_error)
  : error(result_error)
{
  for (auto d:result_data) {
    this->data.push_back(d);
  }
}

// returns true if the two strings are equal when lowercase
bool CompareLowercaseStrings(const std::string& a, const std::string& b) {
  if (a.size() != b.size()) {
    return false;
  }

  // compare character by character
  for (size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(a[i]) != std::tolower(b[i])) {
      return false;
    }
  }

  return true;
}

// Before capturing XML data, we must have seen a preceding element whose content contains a certain substring
// Once the preceding element has been seen, the requested data can be found in the data of the specified containing element, ordered by names
// possible to have multiple pieces of data meet the above criteria, but once we exit out of the last containing element, we mark it done
class XMLData {
  public:
    // Constructor
    XMLData(
      const std::string& prec_elem_name,
      const std::string& prec_elem_data_substr,
      const std::vector<std::string>& cont_elem_names)
    // all detection flags start off
    : precending_element_name(prec_elem_name),
      preceding_element_in_progress(false),
      precending_element_data_substring(prec_elem_data_substr),
      preceding_element_met(false),
      containing_elements_met(0),
      desired_data_in_progress(false),
      desired_data_done(false)
    {
      this->containing_element_names.resize(0);
      for (auto n:cont_elem_names) {
        this->containing_element_names.push_back(n);
      }

      this->desired_data.resize(0);
    }

    // As an element is started, see if it requires changes to our data-storing state
    void StartElement(const std::string& element_name) {
      if (!this->preceding_element_met) { // phase 1: before we've seen the preceding element
        if (CompareLowercaseStrings(element_name, this->precending_element_name)) {
          this->preceding_element_in_progress = true;
        }
      }
      else if (!this->desired_data_done) { // phase 3: getting into the containing elements
        if (this->containing_elements_met < this->containing_element_names.size()) {
          if (CompareLowercaseStrings(element_name, this->containing_element_names[this->containing_elements_met])) {
            ++this->containing_elements_met;
            if (this->containing_elements_met == this->containing_element_names.size()) {
              this->desired_data_in_progress = true;
            }
          }
        }
        else { // unexpected nested containing element, so we must reset
          this->containing_elements_met = 0;
        }
      }
    }

    void CharacterReceiver(const xmlChar* c, int c_count) {
      if (this->preceding_element_in_progress) { // phase 2: while in the preceding element
        std::string chars = std::string(reinterpret_cast<const char*>(c), c_count);
        if (chars.find(this->precending_element_data_substring) != std::string::npos) {
          this->preceding_element_met = true;
        }
      }
      else if (this->desired_data_in_progress && this->containing_elements_met == this->containing_element_names.size()) { // phase 4: inside the innermost containing elements (the desired data)
        this->desired_data.push_back(std::string(reinterpret_cast<const char*>(c), c_count));
      }
    }

    void EndElement(const std::string& element_name) {
      if (this->preceding_element_in_progress && CompareLowercaseStrings(element_name, this->precending_element_name)) {
        this->preceding_element_in_progress = false;
      }
      else if (!this->desired_data_done) { // phase 5: getting out of the containing elements
        if (this->containing_elements_met > 0 && CompareLowercaseStrings(element_name, this->containing_element_names[this->containing_elements_met-1])) {
          --this->containing_elements_met;
          if (this->desired_data_in_progress && this->containing_elements_met == 0) {
            this->desired_data_in_progress = false;
            this->desired_data_done = true;
          }
        }
      }
    }

    const std::vector<std::string> DesiredData() const {
      return this->desired_data;
    }

  private:
    std::string precending_element_name;
    bool preceding_element_in_progress;
    std::string precending_element_data_substring;
    bool preceding_element_met;

    std::vector<std::string> containing_element_names;
    unsigned int containing_elements_met;

    std::vector<std::string> desired_data;
    bool desired_data_in_progress;
    bool desired_data_done;
};

// http://xmlsoft.org/html/libxml-parser.html#startElementSAXFunc
void StartXMLElement(void* user_data, const xmlChar* element_name, const xmlChar** name_val_attribute_array) {
  // Cast to our user data type
  XMLData* user_xml_data = static_cast<XMLData*>(user_data);
  user_xml_data->StartElement(std::string(reinterpret_cast<const char*>(element_name)));

  // tell the compiler that we intend to not use the attribute array: https://stackoverflow.com/questions/1043034/what-does-void-mean-in-c-c-and-c
  (void) name_val_attribute_array;
}

// http://xmlsoft.org/html/libxml-parser.html#charactersSAXFunc
void CharacterReceiver(void* user_data, const xmlChar* c, int c_count) {
  // Cast to our user data type
  XMLData* user_xml_data = static_cast<XMLData*>(user_data);
  user_xml_data->CharacterReceiver(c, c_count);
}

// http://xmlsoft.org/html/libxml-parser.html#endElementSAXFunc
void EndXMLElement(void* user_data, const xmlChar* element_name) {
  // Cast to our user data type
  XMLData* user_xml_data = static_cast<XMLData*>(user_data);
  user_xml_data->EndElement(std::string(reinterpret_cast<const char*>(element_name)));
}

// http://xmlsoft.org/html/libxml-tree.html#xmlSAXHandler
htmlSAXHandler sax_handler = htmlSAXHandler{
  NULL,               // internalSubsetSAXFunc
  NULL,               // isStandaloneSAXFunc
  NULL,		  			    // hasInternalSubsetSAXFunc
  NULL,	  						// hasExternalSubsetSAXFunc
  NULL, 							// resolveEntitySAXFunc
  NULL,							  // getEntitySAXFunc
  NULL,						  	// entityDeclSAXFunc
  NULL,					  		// notationDeclSAXFunc
  NULL,				  			// attributeDeclSAXFunc
  NULL,			  				// elementDeclSAXFunc
  NULL,		  					// unparsedEntityDeclSAXFunc
  NULL,	  						// setDocumentLocatorSAXFunc
  NULL, 							// startDocumentSAXFunc
  NULL,						  	// endDocumentSAXFunc
  StartXMLElement,    // startElementSAXFunc
  EndXMLElement,      // endElementSAXFunc
  NULL,							  // referenceSAXFunc
  CharacterReceiver,  // charactersSAXFunc
  NULL,					  		// ignorableWhitespaceSAXFunc
  NULL,						  	// processingInstructionSAXFunc
  NULL,							  // commentSAXFunc
  NULL, 							// warningSAXFunc
  NULL,	  						// errorSAXFunc
  NULL,		  					// fatalErrorSAXFunc
  NULL,			  				// getParameterEntitySAXFunc
  CharacterReceiver,  // cdataBlockSAXFunc
  NULL,							  // externalSubsetSAXFunc
  0,                  // initialized
  NULL,               // _private
  NULL,               // startElementNs
  NULL,               // endElementNs
  NULL,               // serror
};

XMLParseResult GetPrecededAndNestedData(
  const std::string& page_data,
  const std::string& preceding_element_name,
  const std::string& preceding_element_data_substring,
  const std::vector<std::string>& containing_element_names)
{
  htmlParserCtxtPtr parser_context;
  XMLData xml_data = XMLData(preceding_element_name, preceding_element_data_substring, containing_element_names);

  // http://xmlsoft.org/html/libxml-HTMLparser.html#htmlCreatePushParserCtxt
  parser_context = htmlCreatePushParserCtxt(
    &sax_handler,
    &xml_data, // void* type to receive data as we are user
    "", // empty chunk when initializing the parser context
    0, // size of chunk (empty)
    "", // file name, which is irrelevant as we are handling in-memory data
    XML_CHAR_ENCODING_NONE); // encoding is optional to specify; this is the default
  if (parser_context == NULL) {
    return XMLParseResult(std::vector<std::string>(), "failed to create parser context");
  }
  
  // http://xmlsoft.org/html/libxml-HTMLparser.html#htmlParseChunk
  // while this does return a success indicator, we skip checking it because it returns 801 (unknown tag) even when it parses through all elements in the data
  htmlParseChunk(parser_context, page_data.c_str(), static_cast<int>(page_data.size()), 0);
  htmlParseChunk(parser_context, "", 0, 1);

  // http://xmlsoft.org/html/libxml-HTMLparser.html#htmlFreeParserCtxt ; does memory management of the parser for us
  htmlFreeParserCtxt(parser_context);

  return XMLParseResult(xml_data.DesiredData(), "");
}
