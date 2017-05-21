#pragma once
#include <silicium/html/tree.hpp>

namespace Si
{
    namespace html
    {
        namespace tags
        {

            //----------------TITLE tag----------------
            template <class Element>
            inline auto html(Element &&content)
            {
                return tag("html", std::forward<Element>(content));
            }

            template <class Element>
            inline auto head(Element &&content)
            {
                return tag("head", std::forward<Element>(content));
            }

            template <class Element>
            inline auto body(Element &&content)
            {
                return tag("body", std::forward<Element>(content));
            }

            //----------------TITLE tag----------------
            inline auto title(std::string const &content)
            {
                return tag("title", text(content));
            }

            //----------------H1 tag----------------
            template <class Element>
            inline auto h1(Element &&content)
            {
                return tag("h1", std::forward<Element>(content));
            }

            //----------------H2 tag----------------
            template <class Element>
            inline auto h2(Element &&content)
            {
                return tag("h2", std::forward<Element>(content));
            }

            //----------------H3 tag----------------
            template <class Element>
            inline auto h3(Element &&content)
            {
                return tag("h3", std::forward<Element>(content));
            }

            //----------------H4 tag----------------
            template <class Element>
            inline auto h4(Element &&content)
            {
                return tag("h4", std::forward<Element>(content));
            }

            //----------------MENU tag----------------
            template <class Element>
            inline auto menu(Element &&content)
            {
                return tag("menu", std::forward<Element>(content));
            }

            //----------------FOOTER tag----------------
            template <class Element>
            inline auto footer(Element &&content)
            {
                return tag("footer", std::forward<Element>(content));
            }

            //----------------P tag----------------
            inline auto p(std::string const &content)
            {
                return tag("p", text(content));
            }

            template <class Element, class Attributes>
            inline auto p(Attributes &&attributes, Element &&content)
            {
                return tag("p", std::forward<Attributes>(attributes),
                           std::forward<Element>(content));
            }

            //----------------DIV tag----------------
            template <class Element>
            inline auto div(Element &&content)
            {
                return tag("div", std::forward<Element>(content));
            }

            template <class Element, class Attributes>
            inline auto div(Attributes &&attributes, Element &&content)
            {
                return tag("div", std::forward<Attributes>(attributes),
                           std::forward<Element>(content));
            }

            //----------------SPAN tag----------------
            template <class Element>
            auto span(Element &&content)
            {
                return tag("span", std::forward<Element>(content));
            }

            template <class Element, class Attributes>
            inline auto span(Attributes &&attributes, Element &&content)
            {
                return tag("span", std::forward<Attributes>(attributes),
                           std::forward<Element>(content));
            }

            //----------------table tags----------------
            template <class Element>
            inline auto table(Element &&content)
            {
                return tag("table", std::forward<Element>(content));
            }

            template <class Element>
            inline auto table(std::string const &summary, Element &&content)
            {
                return tag("table", Si::html::attribute("summary", summary),
                           std::forward<Element>(content));
            }

            template <class Element>
            auto thead(Element &&content)
            {
                return tag("thead", std::forward<Element>(content));
            }

            template <class Element>
            auto th(Element &&content)
            {
                return tag("th", std::forward<Element>(content));
            }

            template <class Element>
            inline auto tbody(Element &&content)
            {
                return tag("tbody", std::forward<Element>(content));
            }

            template <class Element>
            inline auto tfoot(Element &&content)
            {
                return tag("tfoot", std::forward<Element>(content));
            }

            //----------------TR tag----------------
            template <class Element>
            auto tr(Element &&content)
            {
                return tag("tr", std::forward<Element>(content));
            }

            //----------------TD tag----------------
            template <class Element>
            inline auto td(Element &&content)
            {
                return tag("td", std::forward<Element>(content));
            }

            //----------------UL tag----------------
            template <class Element>
            inline auto ul(Element &&content)
            {
                return tag("ul", std::forward<Element>(content));
            }

            template <class Element, class Attributes>
            auto ul(Attributes &&attributes, Element &&content)
            {
                return tag("ul", std::forward<Attributes>(attributes),
                           std::forward<Element>(content));
            }

            //----------------LI tag----------------
            template <class Element>
            inline auto li(Element &&content)
            {
                return tag("li", std::forward<Element>(content));
            }

            //----------------PRE tag----------------
            template <class Element>
            inline auto pre(Element &&content)
            {
                return tag("pre", std::forward<Element>(content));
            }

            template <class Element, class Attributes>
            inline auto pre(Attributes &&attributes, Element &&content)
            {
                return tag("pre", std::forward<Attributes>(attributes),
                           std::forward<Element>(content));
            }

            template <class Attributes>
            inline auto br(Attributes &&attributes)
            {
                return tag("br", std::forward<Attributes>(attributes), empty);
            }

            //----------------classes attrib----------------
            inline auto cl(std::string const &content)
            {
                return Si::html::attribute("class", content);
            }

            //----------------href attrib----------------
            inline auto href(std::string const &link)
            {
                return Si::html::attribute("href", link);
            }

            // Opens the link in a new tab
            inline auto href_new_tab(std::string const &link)
            {
                return href(link) + Si::html::attribute("target", "_blank");
            }

            template <class Element, class Attributes>
            inline auto a(Attributes &&attributes, Element &&content)
            {
                return tag("a", std::forward<Attributes>(attributes),
                           std::forward<Element>(content));
            }

            // PSEUDO TAG: link (emulates the a-tag)
            template <std::size_t N>
            inline auto link(std::string const &protocol,
                             char const(&address_without_protocol)[N],
                             std::string const &caption)
            {
                return dynamic(
                    [protocol, &address_without_protocol,
                     caption](code_sink &sink)
                    {
                        if (protocol == "http://" || protocol == "https://")
                        {
                            tag("a", href_new_tab(protocol +
                                                  address_without_protocol),
                                text(caption))
                                .generate(sink);
                        }
                        else
                        {
                            tag("a", href(protocol + address_without_protocol),
                                text(caption))
                                .generate(sink);
                        }
                    });
            }

            template <std::size_t N>
            inline auto link(std::string const &protocol,
                             char const(&address_without_protocol)[N])
            {
                return link(protocol, address_without_protocol,
                            address_without_protocol);
            }

            // PSEUDO ATTRIBUTE: anchor_attributes (emulates a jump mark on a
            // page)
            template <std::size_t N>
            inline auto anchor_attributes(char const(&name)[N])
            {
                return attribute("name", name) + href(std::string("#") + name);
            }
        }
    }
}
