/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.tree_view;

namespace lysa::ui {

    TreeView::TreeView(): Widget(TREEVIEW) {}

    void TreeView::setResources(const std::string& resBox, const std::string& resScroll, const std::string&) {
        if (box == nullptr) {
            box = std::make_shared<Box>();
            vScroll = std::make_shared<VScrollBar>(0.0f, 0.0f);
            add(vScroll, RIGHT, resScroll);
            add(box, FILL, resBox);
            box->setDrawBackground(false);
            box->setPadding(1);
            vScroll->setStep(2);
        }
    }

    void TreeView::removeAllItems() {
        box->removeAll();
        items.clear();
    }

    std::shared_ptr<TreeView::Item>& TreeView::addItem(std::shared_ptr<Widget> item) {
        items.push_back(std::make_shared<Item>(item));
        auto& newWidget = items.back();
        box->add(newWidget, TOPLEFT);
        newWidget->handle = std::make_shared<Text>(" ");
        newWidget->add(newWidget->handle, LEFT, treeTabsSize);
        newWidget->add(item, LEFT);
        newWidget->_setSize(1000.0f, item->getHeight());
        newWidget->setDrawBackground(false);
        return newWidget;
    }

    std::shared_ptr<TreeView::Item>& TreeView::addItem(const std::shared_ptr<Item>& parent, std::shared_ptr<Widget> item) const {
        parent->children.push_back(std::make_shared<Item>(item));
        auto& newWidget = parent->children.back();
        newWidget->level = parent->level + 1;
        box->add(newWidget, TOPLEFT);
        for (int i = 0; i <= newWidget->level; i++) {
            if (i == (newWidget->level)) {
                newWidget->handle = std::make_shared<Text>(" ");
                newWidget->add(newWidget->handle, LEFT, treeTabsSize);
            }
            else {
                newWidget->add(std::make_shared<Panel>(), LEFT, treeTabsSize);
            }
        }
        parent->handle->setText("-");
        newWidget->add(item, LEFT);
        newWidget->_setSize(1000.0f, item->getHeight());
        newWidget->setDrawBackground(false);
        //expand(parent->item);
        return newWidget;
    }

    void TreeView::expand(const std::shared_ptr<Widget>& item) const {
        for (const auto& widget : box->_getChildren()) {
            if (const auto itemWidget = dynamic_cast<Item*>(item.get())) {
                if (itemWidget->item == item) {
                    throw Exception("not implemented");
                    // log("found");
                }
            }
        }
    }

}
