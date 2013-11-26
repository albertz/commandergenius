/* XmlGui - xml parsing class for guichan (http://guichan.sourceforge.net)
 * author - quard (quard8@gmail.com)
 * Changes:
 * v. 0.4   - fixed bug in Visual Studio .NET 2003, when compiler option /GR is not enabled;
              fixed widget allocation, when attribute "name" is't exists
			  deleted all "return false" after GCN_EXCEPTION, because they not used ever
 * v. 0.3.5 - fixed stupid bug, when widgets not parsed if parent widget is NULL
 * v. 0.3   - added parsing method for widget, 
 *		      now checking widget node is in one function
 * v. 0.2   - small bug fix
 *          - added parsing widgets under 'container'
 *		    - changed sscanf to atoi
 * v. 0.1   - first public release
*/


#ifndef XML_GUI_H
#define XML_GUI_H

#include <guichan.hpp>
#include "tinyxml.h"
#include <map>

//!XmlGui class for loading gui widget from xml file
class XmlGui
{
public:
	//!constructor
	XmlGui();
	//!parse xml file
	//@param filename the file name to parse
	//@return true if loaded ok
	bool parse(const std::string &filename);

	//@param name th widget name
	//@return widget by name
	gcn::Widget *getWidget(const std::string &name);

	//!parse default parameters for all widgets
	//@param element the xml element
	//@param widget the current widget
	void parseDefaults(TiXmlElement *element, gcn::Widget *widget);

	//!parse container widget
	//@param element the xml element
	//@param widget the current widget
	void parseContainer(TiXmlElement *element, gcn::Widget *parent);

	//!parse label widget
	//@param element the xml element
	//@param widget the current widget
	void parseLabel(TiXmlElement *element, gcn::Widget *parent);

	//!parse button widget
	//@param element the xml element
	//@param widget the current widget
	void parseButton(TiXmlElement *element, gcn::Widget *parent);

	//!parse checkbox widget
	//@param element the xml element
	//@param widget the current widget
	void parseCheckBox(TiXmlElement *element, gcn::Widget *parent);

	//!parse radiobutton widget
	//@param element the xml element
	//@param widget the current widget
	void parseRadioButton(TiXmlElement *element, gcn::Widget *parent);

	//!parse icon widget
	//@param element the xml element
	//@param widget the current widget
	void parseIcon(TiXmlElement *element, gcn::Widget *parent);

	//!parse textbox widget
	//@param element the xml element
	//@param widget the current widget
	void parseTextBox(TiXmlElement *element, gcn::Widget *parent);

	//!parse textfield widget
	//@param element the xml element
	//@param widget the current widget
	void parseTextField(TiXmlElement *element, gcn::Widget *parent);

	//!parse slider widget
	//@param element the xml element
	//@param widget the current widget
	void parseSlider(TiXmlElement *element, gcn::Widget *parent);

	//!parse window widget
	//@param element the xml element
	//@param widget the current widget
	void parseWindow(TiXmlElement *element, gcn::Widget *parent);

	//!parse scrollarea widget
	//@param element the xml element
	//@param widget the current widget
	void parseScrollArea(TiXmlElement *element,gcn::Widget *parent);

	//!parse dropdown widget
	//@param element the xml element
	//@param widget the current widget
	void parseDropdown(TiXmlElement *element,gcn::Widget *parent);

	//!parse listbox widget
	//@param element the xml element
	//@param widget the current widget
	void parseListbox(TiXmlElement *element,gcn::Widget *parent);
	
	//!add actionlistener to array
	//@param name the actionlistener name
	//@param al the pointer to actionlistener class
	void addActionListener(const std::string &name,gcn::ActionListener *al);

	//!add font 
	//@param name the font name
	//@param al the pointer to font class
	void addFont(const std::string &name,gcn::Font *font);

	
private:
	//!parse xml node
	//@param element - xml element
	//@param parent - the parent widget
	void parseWidgets(TiXmlElement *element, gcn::Widget *parent);

	//!adding widget to parent
	//!parent widget can be Container,ScrollArea,Window. this function get class and set widget or add widget (for Container)
	//@param widget our widget
	//@param parent the parent widget
	void addToParent(gcn::Widget *widget, gcn::Widget *parent);

	//!check string value for boolean value
	//@param value the string value
	//@return true if value are "1" or "true"
	bool checkBool(const std::string &value);


	std::map<std::string,gcn::Widget*> widgets;
	std::map<std::string,gcn::ActionListener*> actions;
	std::map<std::string,gcn::Font*> fonts;

	//temporary xml document (need by some functions)
	TiXmlDocument *doc;
};

class XmlListModel : public gcn::ListModel
{
public:
    virtual int getNumberOfElements();
    virtual std::string getElementAt(int i);
	std::vector<std::string> items;
};

#endif

