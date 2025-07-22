/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.text_edit;

namespace lysa::ui {

    TextEdit::TextEdit(const std::string& text):
        Widget{TEXTEDIT},
        text{text} {
    }

    void TextEdit::setText(const std::string& TEXT) {
        if (text == TEXT) return;
        if (text.empty()) {
            selStart = 0;
            startPos = 0;
        }
        text = TEXT;
        computeNDispChar();
        if (parent) { parent->refresh(); }
        textBox->setText(text.substr(startPos, nDispChar + 1));
        box->refresh();
        refresh();
        auto event = EventTextChange{.text = text};
        event.source = this;
        emit(Event::OnTextChange, &event);
    }

    void TextEdit::setSelStart(const uint32 start) {
        selStart = start;
        if (startPos > selStart) {
            startPos = selStart;
        }
    }

    void TextEdit::setResources(const std::string& resource) {
        add(box, FILL, resource);
        box->add(textBox, HCENTER);
        selStart = 0;
        startPos = 0;
        computeNDispChar();
    }

    bool TextEdit::eventKeyDown(Key key) {
        const auto consumed = Widget::eventKeyDown(key);
        if (isReadOnly()) { return key; }

        setFreezed(true);
        if (key == Key::KEY_LEFT) {
            if (selStart > 0) { selStart--; }
        }
        else if (key == Key::KEY_RIGHT) {
            if (selStart < text.length()) { selStart++; }
        }
        else if (key == Key::KEY_END) {
            selStart = text.length();
        }
        else if (key == Key::KEY_HOME) {
            selStart = 0;
        }
        else if (key == Key::KEY_BACKSPACE) {
            if (selStart > 0) {
                selStart--;
                setText(text.substr(0, selStart) + text.substr(selStart + 1,
                                                               text.length() - selStart - 1));
            }
        }
        else if (key == Key::KEY_DELETE) {
            if (selStart < text.length()) {
                setText(text.substr(0, selStart) + text.substr(selStart + 1,
                                                               text.length() - selStart - 1));
            }
        }
        /*else if ((K != keyb->Key::SHIFTRIGHT) &&
                (K != keyb->Key::SHIFTLEFT) &&
                (K != keyb->Key::CTRLRIGHT) &&
                (K != keyb->Key::CTRLLEFT) &&
                (K != keyb->Key::ALTRIGHT) &&
                (K != keyb->Key::ALTLEFT))
        {
            UChar c = keyb->CodeToChar(K);
            if (c >= _WORD(0x0020)) {
                SetText(text.Left(selStart) + c +
                        text.Right(text.Len() - selStart));
                selStart++;
            }
            else {
                Freeze() = FALSE;
                return K;
            }
        }*/
        else {
            setFreezed(false);
            return consumed;
        }
        computeNDispChar();
        if (selStart < startPos) {
            startPos = selStart;
        }
        else if (((selStart + selLen) > (startPos + nDispChar)) &&
            (nDispChar != text.length())) {
            startPos = selStart - nDispChar;
            }
        computeNDispChar();
        setFreezed(false);
        textBox->setText(text.substr(startPos, nDispChar + 1));
        box->refresh();
        refresh();
        return key;
    }
}
