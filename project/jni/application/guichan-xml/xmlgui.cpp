#include "xmlgui.h"
#include <stdio.h>
#include <stdlib.h>

XmlGui::XmlGui()
{
	
}

bool XmlGui::parse(const std::string &filename)
{
	TiXmlElement *element = NULL;
	TiXmlNode *node = NULL;

	doc = new TiXmlDocument(filename.c_str());

	bool loadOkay = doc->LoadFile();

	if ( !loadOkay )
	{
		throw GCN_EXCEPTION("Error parsing xml file.");
	}
	node = doc->FirstChild();

	if(node == NULL)
	{
		throw GCN_EXCEPTION("Xml document is null or has errors.");
	}

	while(node!=NULL)
	{
		element = node->ToElement();
		parseWidgets(element,NULL);
		node = doc->IterateChildren(node);
	}

	return true;
}

void XmlGui::addToParent(gcn::Widget *widget,gcn::Widget *parent)
{
	if(!parent) return;

	gcn::Container* top = dynamic_cast<gcn::Container *>(parent);

	if(top)
	{
		top->add(widget);
	}
	else
	{
		gcn::Window* window = dynamic_cast<gcn::Window *>(parent);

		if(window)
//			window->setContent(widget);
			window->add(widget);
		else
		{
			gcn::ScrollArea* scrollarea = dynamic_cast<gcn::ScrollArea *>(parent);
			scrollarea->setContent(widget);
		}
	}

}

void XmlGui::parseWidgets(TiXmlElement *element, gcn::Widget *parent)
{
	if(!element) return;

	std::string value = element->Value();

	if(value == "container")
		parseContainer(element,parent);
	if(value == "label")
		parseLabel(element,parent);
	else if(value == "radiobutton")
		parseRadioButton(element,parent);
	else if(value == "button")
		parseButton(element,parent);
	else if(value == "checkbox")
		parseCheckBox(element,parent);
	else if(value == "icon")
		parseIcon(element,parent);
	else if(value == "textbox")
		parseTextBox(element,parent);
	else if(value == "textfield")
		parseTextField(element,parent);
	else if(value == "slider")
		parseSlider(element,parent);
	else if(value == "window")
		parseWindow(element,parent);
	else if(value == "scrollarea")
		parseScrollArea(element,parent);
	else if(value == "dropdown")
		parseDropdown(element,parent);
	else if(value == "listbox")
		parseListbox(element,parent);
}

void XmlGui::parseDefaults(TiXmlElement *element, gcn::Widget *widget)
{
	if(!element) return;

	
	if(element->Attribute("x"))
	{
		int x = atoi(element->Attribute("x")->c_str());
		widget->setX(x);
	}


	if(element->Attribute("y"))
	{
		int y = atoi(element->Attribute("y")->c_str());
		widget->setY(y);
	}


	if(element->Attribute("width"))
	{
		int w = atoi(element->Attribute("width")->c_str());
		widget->setWidth(w);
	}

	if(element->Attribute("height"))
	{
		int h = atoi(element->Attribute("height")->c_str());
		widget->setHeight(h);
	}

	if(element->Attribute("basecolor"))
	{
		int color;
		sscanf(element->Attribute("basecolor")->c_str(),"%x",&color);
		widget->setBaseColor(gcn::Color(color));
	}

	if(element->Attribute("foregroundcolor"))
	{
		int color;
		sscanf(element->Attribute("foregroundcolor")->c_str(),"%x",&color);
		widget->setForegroundColor(gcn::Color(color));
	}

	if(element->Attribute("backgroundcolor"))
	{
		int color;
		sscanf(element->Attribute("backgroundcolor")->c_str(),"%x",&color);
		widget->setBackgroundColor(gcn::Color(color));
	}


	if(element->Attribute("frame"))
	{
		int b = atoi(element->Attribute("framesize")->c_str());
		widget->setFrameSize(b);
	}

	if(element->Attribute("font"))
	{
		if(fonts[*element->Attribute("font")])
			widget->setFont(fonts[*element->Attribute("visible")]);
	}
	
	if(element->Attribute("visible"))
		widget->setVisible(checkBool(*element->Attribute("visible")));

	if(element->Attribute("focusable"))
		widget->setFocusable(checkBool(*element->Attribute("focusable")));
	
	if(element->Attribute("enabled"))
		widget->setEnabled(checkBool(*element->Attribute("enabled")));
	
	if(element->Attribute("tabin"))
		widget->setTabInEnabled(checkBool(*element->Attribute("tabin")));

	if(element->Attribute("tabout"))
		widget->setTabOutEnabled(checkBool(*element->Attribute("tabout")));

	if(element->Attribute("eventId"))
		widget->setActionEventId(*element->Attribute("eventId"));

	if(element->Attribute("actionListener"))
		widget->addActionListener(actions[*element->Attribute("actionListener")]);
}

void XmlGui::parseContainer(TiXmlElement *element, gcn::Widget *parent)
{

	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("Container Widget must have a unique name");
	}

	gcn::Container *c = new gcn::Container();


	if(element->Attribute("opaque"))
		c->setOpaque(checkBool(*element->Attribute("opaque")));


	parseDefaults(element,c);

	//parsing child elements
	TiXmlNode *child = element->FirstChild();
	
	if(child)
	{
		while(child)
		{
			TiXmlElement *e = child->ToElement();

			parseWidgets(e,c);

			child = doc->IterateChildren(child);
		}
	}
	
	addToParent(c,parent);
	widgets[name] = c;
}

void XmlGui::parseLabel(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;

	if(element->Attribute("name"))
	{
		name = *element->Attribute("name");
	}else
	{
		throw GCN_EXCEPTION("Label Widget must have a unique name");
	}


	gcn::Label *label = new gcn::Label;


	if(element->Attribute("caption"))
	{
		label->setCaption(*element->Attribute("caption"));
	}

	label->adjustSize();

	if(element->Attribute("align"))
	{
		if(*element->Attribute("align") == "center" || *element->Attribute("align") == "CENTER")
		{
			label->setAlignment(gcn::Graphics::CENTER);
		}else if(*element->Attribute("align") == "left" || *element->Attribute("align") == "LEFT")
		{
			label->setAlignment(gcn::Graphics::LEFT);
		}else
		{
			label->setAlignment(gcn::Graphics::RIGHT);
		}
	}

	parseDefaults(element,label);

	addToParent(label,parent);
	widgets[name] = label;
}

void XmlGui::parseButton(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("Button Widget must have a unique name");
	}

	gcn::Button *button = new gcn::Button;


	if(element->Attribute("caption"))
	{
		button->setCaption(*element->Attribute("caption"));
	}

	if(element->Attribute("align"))
	{
		if(*element->Attribute("align") == "center" || *element->Attribute("align") == "CENTER")
		{
			button->setAlignment(gcn::Graphics::CENTER);
		}else if(*element->Attribute("align") == "left" || *element->Attribute("align") == "LEFT")
		{
			button->setAlignment(gcn::Graphics::LEFT);
		}else
		{
			button->setAlignment(gcn::Graphics::RIGHT);
		}
	}

	button->adjustSize();

	parseDefaults(element,button);

//	button->lostFocus(); //fix: this call must't exists
//?????

	addToParent(button,parent);

	widgets[name] = button;
}

void XmlGui::parseCheckBox(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;

	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("Checkbox Widget must have a unique name");
	}

	gcn::CheckBox *checkbox = new gcn::CheckBox;


	if(element->Attribute("caption"))
	{
		checkbox->setCaption(*element->Attribute("caption"));
	}

	checkbox->adjustSize();


	if(element->Attribute("selected"))
		checkbox->setSelected(checkBool(*element->Attribute("selected")));			

	parseDefaults(element,checkbox);
	

	addToParent(checkbox,parent);

	widgets[name] = checkbox;
}


void XmlGui::parseRadioButton(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("RadioButton Widget must have a unique name");
	}

	gcn::RadioButton *radio = new gcn::RadioButton;

	if(element->Attribute("caption"))
	{
		radio->setCaption(*element->Attribute("caption"));
	}

	radio->adjustSize();


	if(element->Attribute("selected"))
		radio->setSelected(checkBool(*element->Attribute("selected")));			
	
	if(element->Attribute("group"))
		radio->setGroup(*element->Attribute("group"));			
	
	parseDefaults(element,radio);

	addToParent(radio,parent);

	widgets[name] = radio;
}

void XmlGui::parseIcon(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("Icon Widget must have a unique name");
	}

	gcn::Icon *icon;

	gcn::Image *image;

	if(element->Attribute("image"))
	{
//		image = new gcn::Image(*element->Attribute("image"));
		image = gcn::Image::load(*element->Attribute("image"));
	}

	if(image)
	{
		icon = new gcn::Icon(image);
	}

	parseDefaults(element,icon);

	addToParent(icon,parent);
	widgets[name] = icon;
}


void XmlGui::parseTextBox(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("TextBox Widget must have a unique name");
	}

	gcn::TextBox *textbox = new gcn::TextBox;

	if(element->Attribute("editable"))
		textbox->setEditable(checkBool(*element->Attribute("editable")));

	if(element->Attribute("text"))
	{
		textbox->setText(*element->Attribute("text"));
	}

	if(element->Attribute("opaque"))
		textbox->setOpaque(checkBool(*element->Attribute("opaque")));

	parseDefaults(element,textbox);

	addToParent(textbox,parent);

	widgets[name] = textbox;
}

void XmlGui::parseTextField(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("TextField Widget must have a unique name");
	}

	gcn::TextField *textfield = new gcn::TextField;

	if(element->Attribute("text"))
	{
		textfield->setText(*element->Attribute("text"));
	}

	parseDefaults(element,textfield);

	addToParent(textfield,parent);

	widgets[name] = textfield;
}

void XmlGui::parseSlider(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("Slider Widget must have a unique name");
	}

	gcn::Slider *slider = new gcn::Slider;

	if(element->Attribute("start"))
	{
		int start  = atoi(element->Attribute("start")->c_str());
		
		slider->setScaleStart(start);
	}

	if(element->Attribute("end"))
	{
		int end = atoi(element->Attribute("end")->c_str());
		slider->setScaleEnd(end);
	}

	if(element->Attribute("value"))
	{
		int value = atoi(element->Attribute("value")->c_str());
		slider->setValue(value);
	}

	if(element->Attribute("markerLength"))
	{
		int l = atoi(element->Attribute("markerLength")->c_str());
		slider->setMarkerLength(l);
	}

	if(element->Attribute("stepLength"))
	{
		int l = atoi(element->Attribute("stepLength")->c_str());
		slider->setStepLength(l);
	}

	if(element->Attribute("orientation"))
	{
		if(*element->Attribute("orientation") == "HORIZONTAL" || *element->Attribute("orientation") == "horizontal")
			slider->setOrientation(gcn::Slider::HORIZONTAL);
		else slider->setOrientation(gcn::Slider::VERTICAL);
	}



	parseDefaults(element,slider);

	addToParent(slider,parent);

	widgets[name] = slider;
}

void XmlGui::parseWindow(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = *element->Attribute("name");
	}else
	{
		throw GCN_EXCEPTION("Window Widget must have a unique name");
	}

	gcn::Window *window = new gcn::Window;


	if(element->Attribute("caption"))
	{
		window->setCaption(*element->Attribute("caption"));
	}

	if(element->Attribute("tabbing"))
	{
		window->setPadding(checkBool(*element->Attribute("tabbing")));
	}

	if(element->Attribute("movable"))
	{
		window->setMovable(checkBool(*element->Attribute("movable")));
	}

	if(element->Attribute("titleBarHeight"))
	{
		int h = atoi(element->Attribute("titleBarHeight")->c_str());
		window->setTitleBarHeight(h);
	}


	if(element->Attribute("opaque"))
		window->setOpaque(checkBool(*element->Attribute("opaque")));


	parseDefaults(element,window);

	//parsing child elements
	TiXmlNode *child = element->FirstChild();

	if(child)
	{
		while(child)
		{
			TiXmlElement *e = child->ToElement();
			parseWidgets(e,window);
			child = doc->IterateChildren(child);
		}
	}


	window->resizeToContent();

	addToParent(window,parent);

	widgets[name] = window;
}


void XmlGui::parseScrollArea(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("ScrollArea Widget must have a unique name");
	}

	gcn::ScrollArea *scroll = new gcn::ScrollArea;


	if(element->Attribute("hPolicy"))
	{
		if(*element->Attribute("hPolicy") == "ALWAYS" || *element->Attribute("hPolicy") == "always")
			scroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
		else if(*element->Attribute("hPolicy") == "NEVER" || *element->Attribute("hPolicy") == "never")
			scroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
	}

	if(element->Attribute("vPolicy"))
	{
		if(*element->Attribute("vPolicy") == "ALWAYS" || *element->Attribute("vPolicy") == "always")
			scroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
		else if(*element->Attribute("vPolicy") == "NEVER" || *element->Attribute("vPolicy") == "never")
			scroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
	}

	if(element->Attribute("vScrollAmount"))
	{
		int h;
		sscanf(element->Attribute("vScrollAmount")->c_str(),"%d",&h);
		scroll->setVerticalScrollAmount(h);
	}

	if(element->Attribute("hScrollAmount"))
	{
		int h = atoi(element->Attribute("hScrollAmount")->c_str());
		scroll->setHorizontalScrollAmount(h);
	}

	if(element->Attribute("scrollBarWidth"))
	{
		int w = atoi(element->Attribute("scrollbarWidth")->c_str());
		scroll->setScrollbarWidth(w);
	}

	if(element->Attribute("content"))
	{
		gcn::Widget *content = getWidget(*element->Attribute("content"));
		if(content)
		{
			scroll->setContent(content);
		}
	}

	parseDefaults(element,scroll);

	if(!scroll->getContent())
	{

		//parsing child elements
		TiXmlNode *child = element->FirstChild();

		if(child)
		{
			while(child)
			{
				TiXmlElement *e = child->ToElement();

				parseWidgets(e,scroll);

				child = doc->IterateChildren(child);
			}
		}
	}


	addToParent(scroll,parent);

	widgets[name] = scroll;
}

void XmlGui::parseDropdown(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("DropDown Widget must have a unique name");
	}

	gcn::DropDown *dropdown = new gcn::DropDown;


	parseDefaults(element,dropdown);

	XmlListModel *listmodel;
	//parsing child elements
	TiXmlNode *child = element->FirstChild();

	if(child)
	{

		listmodel = new XmlListModel;

		while(child)
		{
			TiXmlElement *e = child->ToElement();
			if(e->Value() == "li")
			{
				listmodel->items.push_back(e->FirstChild()->Value());
			}
			child = doc->IterateChildren(child);
		}
	}	
	
	if(listmodel)
		dropdown->setListModel(listmodel);

	if(element->Attribute("selected"))
	{
		int s = atoi(element->Attribute("selected")->c_str());
		dropdown->setSelected(s);
	}

	addToParent(dropdown,parent);
	widgets[name] = dropdown;
}

void XmlGui::parseListbox(TiXmlElement *element,gcn::Widget *parent)
{
	if(!element)
		return;


	std::string name;
	if(element->Attribute("name"))
	{
		name = element->Attribute("name")->c_str();
	}else
	{
		throw GCN_EXCEPTION("ListBox Widget must have a unique name");
	}

	gcn::ListBox *listbox = new gcn::ListBox;


	parseDefaults(element,listbox);

	XmlListModel *listmodel;
	//parsing child elements
	TiXmlNode *child = element->FirstChild();

	if(child)
	{

		listmodel = new XmlListModel;

		while(child)
		{
			TiXmlElement *e = child->ToElement();
			if(e->Value() == "li")
			{
				listmodel->items.push_back(e->FirstChild()->Value());
			}
			child = doc->IterateChildren(child);
		}
	}	
	
	if(listmodel)
		listbox->setListModel(listmodel);

	if(element->Attribute("selected"))
	{
		int s = atoi(element->Attribute("selected")->c_str());
		listbox->setSelected(s);
	}

	addToParent(listbox,parent);
	widgets[name] = listbox;
}


gcn::Widget *XmlGui::getWidget(const std::string &name)
{
	
	std::map<std::string,gcn::Widget*>::iterator it = widgets.find(name);
	if(it == widgets.end()) {
		return NULL;
	}

	return widgets[name];
}

bool XmlGui::checkBool(const std::string &value)
{
	return (value == "1" || value == "true" || value == "True");
}

void XmlGui::addActionListener(const std::string &name, gcn::ActionListener *al)
{
	if(al)
		actions[name] = al;
}

void XmlGui::addFont(const std::string &name, gcn::Font *font)
{
	if(font)
		fonts[name] = font;
}


int XmlListModel::getNumberOfElements()
{
	return items.size();
}

std::string XmlListModel::getElementAt(int i)
{
	return items[i];
}

