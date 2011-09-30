// Begin CVS Header
//   $Source: /fs/turing/cvs/copasi_dev/copasi/layoutUI/CQGLLayoutViewer.h,v $
//   $Revision: 1.1 $
//   $Name: Build-33 $
//   $Author: gauges $
//   $Date: 2010/03/10 12:33:51 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef CQLAYOUTVIEWER_H__
#define CQLAYOUTVIEWER_H__

#include <QFrame>

class QScrollBar;
class CQGLLayoutPainter;
class CCopasiDataModel;
class CLayout;
class CLRenderInformationBase;
/**
 * This class is supposed to act as a sort of scrollview for a
 * CQGLNetworkPainter.
 */
class CQGLLayoutViewer : public QFrame
{
  Q_OBJECT

protected:
  QScrollBar* mpVerticalScrollbar;
  QScrollBar* mpHorizontalScrollbar;
  CQGLLayoutPainter* mpLayoutPainter;

public:
  /**
   * Constructor.
   */
  CQGLLayoutViewer(QWidget* pParent = 0, Qt::WindowFlags f = 0);

  /**
   * Destructor.
   */
  virtual ~CQGLLayoutViewer();

  /**
   * Sets the zoom factor on the network painter and updates the
   *scrollbars.
   */
  void setZoomFactor(double zoom);

  void update(const CCopasiDataModel* pDataModel, CLayout* pLayout, const CLRenderInformationBase* pRenderInfo, const QString& baseDir);

  /**
   * The default style indicates that the passed in render information is a default style.
   * The renderer uses this information to determine whether roles for species reference glyphs
   * need to be deduced.
   */
  void change_style(const CLRenderInformationBase* pRenderInfo, bool defaultStyle = false);

  /**
   * This method tells the painter to temporarily revert the deduced
   * object roles.
   * The painter will however remember the rolls and a call to restoreDeducedRoles
   * will restore them.
   */
  void revertDeducedRoles();

  /**
   * Restores te deduced object roles after they have been temporarily reverted by a call to
   * revertDeducedRoles.
   */
  void restoreDeducedRoles();

  /**
   * Returns a pointer to the OpenGL painter.
   */
  CQGLLayoutPainter* getPainter();

  /**
   * Returns a const pointer to the OpenGL painter.
   */
  const CQGLLayoutPainter* getPainter() const;

protected:
  virtual void resizeEvent(QResizeEvent* e);

  void updateScrollbars();

  void resetView();

protected slots:
  void slotVValueChanged(int value);
  void slotHValueChanged(int value);

  /**
   * This slot is called when the layout painter changes
   * the document (layout).
   */
  void slotDocumentChanged();

  /**
   * This slot is called when the selection changes.
   * The value determines whether a single curve is selected or not.
   */
  void slotSingleCurveSelected(bool selected);

public slots:
  void updateWidget();

  /**
   * This slot is called when the user initiates the action to revert the
   * currently selected curve.
   */
  void slotRevertCurve();

signals:
  void documentChanged();
  void singleCurveSelected(bool selected);
};

#endif /* CQLAYOUTVIEWER_H__ */