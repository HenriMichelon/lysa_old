/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.tree_view;

import lysa.enums;
import lysa.exception;

import lysa.ui.box;
import lysa.ui.panel;
import lysa.ui.scroll_bar;
import lysa.ui.text;
import lysa.ui.widget;

export namespace lysa::ui {

    class TreeView : public Widget {
    public:
        class Item : public Panel {
        public:
            std::shared_ptr<Widget> item;
            std::shared_ptr<Text> handle;
            std::list<std::shared_ptr<Item>> children;
            int level{0};
            bool selected{false};
            bool expanded{false};

            explicit Item(std::shared_ptr<Widget> item) : item{item} {}
        };

        TreeView();

        void setResources(const std::string& resBox, const std::string& resScroll, const std::string&);

        void removeAllItems();

        std::shared_ptr<Item>& addItem(std::shared_ptr<Widget> item);

        std::shared_ptr<Item>& addItem(const std::shared_ptr<Item>& parent, std::shared_ptr<Widget> item) const;

        void expand(const std::shared_ptr<Widget>& item) const;

    private:
        const std::string treeTabsSize{"5,5"};
        float innerHeight;
        float itemsHeight;
        std::list<std::shared_ptr<Item>> items;
        std::shared_ptr<Box> box;
        std::shared_ptr<VScrollBar> vScroll;
    };
}
