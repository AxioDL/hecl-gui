#include "EscapeSequenceParser.hpp"
#include <QFontDatabase>

/* TODO: more complete Vt102 emulation */
// based on information: http://en.m.wikipedia.org/wiki/ANSI_escape_code
// http://misc.flogisoft.com/bash/tip_colors_and_formatting
// http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
void ParseEscapeSequence(int attribute, QListIterator<QString>& i, QTextCharFormat& textCharFormat,
                         const QTextCharFormat& defaultTextCharFormat) {
  switch (attribute) {
  case 0: { // Normal/Default (reset all attributes)
    textCharFormat = defaultTextCharFormat;
    break;
  }
  case 1: { // Bold/Bright (bold or increased intensity)
    textCharFormat.setFontWeight(QFont::Bold);
    break;
  }
  case 2: { // Dim/Faint (decreased intensity)
    textCharFormat.setFontWeight(QFont::Light);
    break;
  }
  case 3: { // Italicized (italic on)
    textCharFormat.setFontItalic(true);
    break;
  }
  case 4: { // Underscore (single underlined)
    textCharFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    textCharFormat.setFontUnderline(true);
    break;
  }
  case 5: { // Blink (slow, appears as Bold)
    textCharFormat.setFontWeight(QFont::Bold);
    break;
  }
  case 6: { // Blink (rapid, appears as very Bold)
    textCharFormat.setFontWeight(QFont::Black);
    break;
  }
  case 7: { // Reverse/Inverse (swap foreground and background)
    QBrush foregroundBrush = textCharFormat.foreground();
    textCharFormat.setForeground(textCharFormat.background());
    textCharFormat.setBackground(foregroundBrush);
    break;
  }
  case 8: { // Concealed/Hidden/Invisible (usefull for passwords)
    textCharFormat.setForeground(textCharFormat.background());
    break;
  }
  case 9: { // Crossed-out characters
    textCharFormat.setFontStrikeOut(true);
    break;
  }
  case 10: { // Primary (default) font
    textCharFormat.setFont(defaultTextCharFormat.font());
    break;
  }
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19: {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString fontFamily = textCharFormat.fontFamily();
    QStringList fontStyles = QFontDatabase::styles(fontFamily);
    int fontStyleIndex = attribute - 11;
    if (fontStyleIndex < fontStyles.length()) {
      textCharFormat.setFont(
          QFontDatabase::font(fontFamily, fontStyles.at(fontStyleIndex), textCharFormat.font().pointSize()));
    }
#else
    QFontDatabase fontDatabase;
    QString fontFamily = textCharFormat.fontFamily();
    QStringList fontStyles = fontDatabase.styles(fontFamily);
    int fontStyleIndex = attribute - 11;
    if (fontStyleIndex < fontStyles.length()) {
      textCharFormat.setFont(
          fontDatabase.font(fontFamily, fontStyles.at(fontStyleIndex), textCharFormat.font().pointSize()));
    }
#endif
    break;
  }
  case 20: { // Fraktur (unsupported)
    break;
  }
  case 21: { // Set Bold off
    textCharFormat.setFontWeight(QFont::Normal);
    break;
  }
  case 22: { // Set Dim off
    textCharFormat.setFontWeight(QFont::Normal);
    break;
  }
  case 23: { // Unset italic and unset fraktur
    textCharFormat.setFontItalic(false);
    break;
  }
  case 24: { // Unset underlining
    textCharFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
    textCharFormat.setFontUnderline(false);
    break;
  }
  case 25: { // Unset Blink/Bold
    textCharFormat.setFontWeight(QFont::Normal);
    break;
  }
  case 26: { // Reserved
    break;
  }
  case 27: { // Positive (non-inverted)
    QBrush backgroundBrush = textCharFormat.background();
    textCharFormat.setBackground(textCharFormat.foreground());
    textCharFormat.setForeground(backgroundBrush);
    break;
  }
  case 28: {
    textCharFormat.setForeground(defaultTextCharFormat.foreground());
    textCharFormat.setBackground(defaultTextCharFormat.background());
    break;
  }
  case 29: {
    textCharFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
    textCharFormat.setFontUnderline(false);
    break;
  }
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37: {
    int colorIndex = attribute - 30;
    QColor color;
    if (QFont::Normal < textCharFormat.fontWeight()) {
      switch (colorIndex) {
      case 0: {
        color = Qt::darkGray;
        break;
      }
      case 1: {
        color = Qt::red;
        break;
      }
      case 2: {
        color = Qt::green;
        break;
      }
      case 3: {
        color = Qt::yellow;
        break;
      }
      case 4: {
        color = Qt::blue;
        break;
      }
      case 5: {
        color = Qt::magenta;
        break;
      }
      case 6: {
        color = Qt::cyan;
        break;
      }
      case 7: {
        color = Qt::white;
        break;
      }
      default: { Q_ASSERT(false); }
      }
    } else {
      /* Normally dark colors, but forced to light colors for visibility */
      switch (colorIndex) {
      case 0: {
        color = Qt::darkGray;
        break;
      }
      case 1: {
        color = Qt::red;
        break;
      }
      case 2: {
        color = Qt::green;
        break;
      }
      case 3: {
        color = Qt::yellow;
        break;
      }
      case 4: {
        color = Qt::blue;
        break;
      }
      case 5: {
        color = Qt::magenta;
        break;
      }
      case 6: {
        color = Qt::cyan;
        break;
      }
      case 7: {
        color = Qt::white;
        break;
      }
      default: { Q_ASSERT(false); }
      }
    }
    textCharFormat.setForeground(color);
    break;
  }
  case 38: {
    if (i.hasNext()) {
      bool ok = false;
      int selector = i.next().toInt(&ok);
      Q_ASSERT(ok);
      QColor color;
      switch (selector) {
      case 2: {
        if (!i.hasNext()) {
          break;
        }
        int red = i.next().toInt(&ok);
        Q_ASSERT(ok);
        if (!i.hasNext()) {
          break;
        }
        int green = i.next().toInt(&ok);
        Q_ASSERT(ok);
        if (!i.hasNext()) {
          break;
        }
        int blue = i.next().toInt(&ok);
        Q_ASSERT(ok);
        color.setRgb(red, green, blue);
        break;
      }
      case 5: {
        if (!i.hasNext()) {
          break;
        }
        int index = i.next().toInt(&ok);
        Q_ASSERT(ok);
        if (index >= 0 && index <= 0x07) { // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
          return ParseEscapeSequence(index - 0x00 + 30, i, textCharFormat, defaultTextCharFormat);
        } else if (index >= 0x08 && index <= 0x0F) { // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
          return ParseEscapeSequence(index - 0x08 + 90, i, textCharFormat, defaultTextCharFormat);
        } else if (index >= 0x10 && index <= 0xE7) { // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
          index -= 0x10;
          int red = index % 6;
          index /= 6;
          int green = index % 6;
          index /= 6;
          int blue = index % 6;
          index /= 6;
          Q_ASSERT(index == 0);
          color.setRgb(red, green, blue);
          break;
        } else if (index >= 0xE8 && index <= 0xFF) { // 0xE8-0xFF:  grayscale from black to white in 24 steps
          qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
          color.setRgbF(intensity, intensity, intensity);
          break;
        }
        textCharFormat.setForeground(color);
        break;
      }
      default: { break; }
      }
    }
    break;
  }
  case 39: {
    textCharFormat.setForeground(defaultTextCharFormat.foreground());
    break;
  }
  case 40:
  case 41:
  case 42:
  case 43:
  case 44:
  case 45:
  case 46:
  case 47: {
    int colorIndex = attribute - 40;
    QColor color;
    switch (colorIndex) {
    case 0: {
      color = Qt::darkGray;
      break;
    }
    case 1: {
      color = Qt::red;
      break;
    }
    case 2: {
      color = Qt::green;
      break;
    }
    case 3: {
      color = Qt::yellow;
      break;
    }
    case 4: {
      color = Qt::blue;
      break;
    }
    case 5: {
      color = Qt::magenta;
      break;
    }
    case 6: {
      color = Qt::cyan;
      break;
    }
    case 7: {
      color = Qt::white;
      break;
    }
    default: { Q_ASSERT(false); }
    }
    textCharFormat.setBackground(color);
    break;
  }
  case 48: {
    if (i.hasNext()) {
      bool ok = false;
      int selector = i.next().toInt(&ok);
      Q_ASSERT(ok);
      QColor color;
      switch (selector) {
      case 2: {
        if (!i.hasNext()) {
          break;
        }
        int red = i.next().toInt(&ok);
        Q_ASSERT(ok);
        if (!i.hasNext()) {
          break;
        }
        int green = i.next().toInt(&ok);
        Q_ASSERT(ok);
        if (!i.hasNext()) {
          break;
        }
        int blue = i.next().toInt(&ok);
        Q_ASSERT(ok);
        color.setRgb(red, green, blue);
        break;
      }
      case 5: {
        if (!i.hasNext()) {
          break;
        }
        int index = i.next().toInt(&ok);
        Q_ASSERT(ok);
        if (index >= 0x00 && index <= 0x07) { // 0x00-0x07:  standard colors (as in ESC [ 40..47 m)
          return ParseEscapeSequence(index - 0x00 + 40, i, textCharFormat, defaultTextCharFormat);
        } else if (index >= 0x08 && index <= 0x0F) { // 0x08-0x0F:  high intensity colors (as in ESC [ 100..107 m)
          return ParseEscapeSequence(index - 0x08 + 100, i, textCharFormat, defaultTextCharFormat);
        } else if (index >= 0x10 && index <= 0xE7) { // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
          index -= 0x10;
          int red = index % 6;
          index /= 6;
          int green = index % 6;
          index /= 6;
          int blue = index % 6;
          index /= 6;
          Q_ASSERT(index == 0);
          color.setRgb(red, green, blue);
          break;
        } else if (index >= 0xE8 && index <= 0xFF) { // 0xE8-0xFF:  grayscale from black to white in 24 steps
          qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
          color.setRgbF(intensity, intensity, intensity);
        }
      }
        textCharFormat.setBackground(color);
        break;
      }
    }
    break;
  }
  case 49: {
    textCharFormat.setBackground(defaultTextCharFormat.background());
    break;
  }
  case 90:
  case 91:
  case 92:
  case 93:
  case 94:
  case 95:
  case 96:
  case 97: {
    int colorIndex = attribute - 90;
    QColor color;
    switch (colorIndex) {
    case 0: {
      color = Qt::darkGray;
      break;
    }
    case 1: {
      color = Qt::red;
      break;
    }
    case 2: {
      color = Qt::green;
      break;
    }
    case 3: {
      color = Qt::yellow;
      break;
    }
    case 4: {
      color = Qt::blue;
      break;
    }
    case 5: {
      color = Qt::magenta;
      break;
    }
    case 6: {
      color = Qt::cyan;
      break;
    }
    case 7: {
      color = Qt::white;
      break;
    }
    default: { Q_ASSERT(false); }
    }
    // color.setRedF(color.redF() * 0.8);
    // color.setGreenF(color.greenF() * 0.8);
    // color.setBlueF(color.blueF() * 0.8);
    textCharFormat.setForeground(color);
    break;
  }
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
  case 106:
  case 107: {
    int colorIndex = attribute - 100;
    QColor color;
    switch (colorIndex) {
    case 0: {
      color = Qt::darkGray;
      break;
    }
    case 1: {
      color = Qt::red;
      break;
    }
    case 2: {
      color = Qt::green;
      break;
    }
    case 3: {
      color = Qt::yellow;
      break;
    }
    case 4: {
      color = Qt::blue;
      break;
    }
    case 5: {
      color = Qt::magenta;
      break;
    }
    case 6: {
      color = Qt::cyan;
      break;
    }
    case 7: {
      color = Qt::white;
      break;
    }
    default: { Q_ASSERT(false); }
    }
    // color.setRedF(color.redF() * 0.8);
    // color.setGreenF(color.greenF() * 0.8);
    // color.setBlueF(color.blueF() * 0.8);
    textCharFormat.setBackground(color);
    break;
  }
  default: { break; }
  }
}

void ReturnInsert(QTextCursor& cur, const QString& text) {
  const auto DoLine = [&](const QString& line) {
    const auto DoReturn = [&](const QString& ret) {
      if (!ret.isEmpty()) {
        cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, ret.size());
        cur.insertText(ret);
      }
    };
    const QStringList list = line.split(QLatin1Char{'\r'});
    DoReturn(list.front());
    if (list.size() > 1) {
      for (auto it = list.begin() + 1; it != list.end(); ++it) {
        cur.movePosition(QTextCursor::StartOfBlock);
        DoReturn(*it);
      }
    }
  };

#if _WIN32
  const QStringList lineSplit = text.split(QStringLiteral("\r\n"));
#else
  const QStringList lineSplit = text.split(QLatin1Char{'\n'});
#endif
  DoLine(lineSplit.front());
  if (lineSplit.size() > 1) {
    for (auto it = lineSplit.begin() + 1; it != lineSplit.end(); ++it) {
      cur.movePosition(QTextCursor::EndOfLine);
      cur.insertBlock();
      DoLine(*it);
    }
  }
}

void ReturnInsert(QTextCursor& cur, const QString& text, const QTextCharFormat& format) {
  const auto DoLine = [&](const QString& line) {
    const auto DoReturn = [&](const QString& ret) {
      if (!ret.isEmpty()) {
        cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, ret.size());
        cur.insertText(ret, format);
      }
    };
    const QStringList list = line.split(QLatin1Char{'\r'});
    DoReturn(list.front());
    if (list.size() > 1) {
      for (auto it = list.begin() + 1; it != list.end(); ++it) {
        cur.movePosition(QTextCursor::StartOfBlock);
        DoReturn(*it);
      }
    }
  };

#if _WIN32
  const QStringList lineSplit = text.split(QStringLiteral("\r\n"));
#else
  const QStringList lineSplit = text.split(QLatin1Char{'\n'});
#endif
  DoLine(lineSplit.front());
  if (lineSplit.size() > 1) {
    for (auto it = lineSplit.begin() + 1; it != lineSplit.end(); ++it) {
      cur.movePosition(QTextCursor::EndOfLine);
      cur.insertBlock();
      DoLine(*it);
    }
  }
}
