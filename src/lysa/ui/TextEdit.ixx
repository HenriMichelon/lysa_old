/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.text_edit;

import lysa.enums;
import lysa.exception;
import lysa.types;

import lysa.ui.box;
import lysa.ui.event;
import lysa.ui.text;
import lysa.ui.widget;

export namespace lysa::ui {

    class TextEdit : public Widget {
    public:
        explicit TextEdit(const std::string& text = "");

        auto isReadOnly() const { return readonly; }

        void setReadOnly(const bool state) { readonly = state; }

        void setText(const std::string& text);

        void setSelStart(uint32 start);

        auto getText() const { return text; }

        auto getSelStart() const { return selStart; }

        auto getFirstDisplayedChar() const { return startPos; }

        // return TRUE if this or parent have keyboard focus
        //bool haveFocus();

        auto getDisplayedText() const { return textBox->getText(); }

        void setResources(const std::string& BRES);

    protected:
        std::string text;
        bool readonly{false};
        uint32 selStart{0};
        uint32 selLen{0};
        uint32 startPos{0};
        uint32 nDispChar{0};
        std::shared_ptr<Box> box;
        std::shared_ptr<Text> textBox;

        bool eventKeyDown(Key key) override;

        // Compute the number of displayed characters
        void computeNDispChar() {
            throw Exception("not implemented");
        }
    };
}
