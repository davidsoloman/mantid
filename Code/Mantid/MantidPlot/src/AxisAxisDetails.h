/**
This class holds the widgets that hold the details for each axis so the contents are only filled once and switching axis only changes a pointer.

@author Keith Brown, Placement Student at ISIS Rutherford Appleton Laboratory from the University of Derby
@date 15/09/2013

Copyright &copy; 2009 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

This file is part of Mantid.

Mantid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Mantid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File change history is stored at: <https://github.com/mantidproject/mantid>
Code Documentation is available at: <http://doxygen.mantidproject.org>
*/

#ifndef AXISAXISDETAILS_H_
#define AXISAXISDETAILS_H_

#include <QWidget>
#include <QList>
class ApplicationWindow;
class QTimeEdit;
class QDateTimeEdit;
class QCheckBox;
class QGroupBox;
class QComboBox;
class QLabel;
class QRadioButton;
class QSpinBox;
class QWidget;
class QTextEdit;
class QPushButton;
class QStringList;
class DoubleSpinBox;
class Graph;
class TextFormatButtons;
class ColorButton;

class AxisAxisDetails: public QWidget
{
  Q_OBJECT
    //details for each axis in the Axis tab
public:
  AxisAxisDetails(ApplicationWindow* app, Graph* graph, int mappedaxis, QWidget *parent = 0); // populate and fill in with existing data
  virtual ~AxisAxisDetails();
  void initWidgets();
  bool modified();
  void apply();
  bool valid();

private slots:
  void enableFormulaBox();
  void showAxis();
  void setLabelFont();
  void setScaleFont();
  void setAxisFormatOptions(int format);
  void setModified();

private:
  bool m_modified, m_initialised;

  ApplicationWindow* d_app;
  Graph* d_graph;

  //formerly *boxShowAxis, *boxShowFormula;
  QCheckBox *chkShowAxis, *chkShowFormula;
  //formerly *labelBox, *boxShowLabels, *leftBox;
  QGroupBox *grpTitle, *grpShowLabels, *grpAxisDisplay;
  //formerly *boxFormula, *boxTitle;
  QTextEdit *txtFormula, *txtTitle;
  //formerly *buttonLabelFont, *btnAxesFont;
  QPushButton *btnLabelFont, *btnAxesFont;
  TextFormatButtons *formatButtons;
  //formerly *boxMajorTicksType, *boxTableName, *boxMinorTicksType, *boxAxisType, *boxFormat, *boxColName;
  QComboBox *cmbMajorTicksType, *cmbTableName, *cmbMinorTicksType, *cmbAxisType, *cmbFormat, *cmbColName;
  //formerly *boxAxisColor, *boxAxisNumColor;
  ColorButton *cbtnAxisColor, *cbtnAxisNumColor;
  //formerly *boxPrecision, *boxAngle, *boxBaseline;
  QSpinBox *spnPrecision, *spnAngle, *spnBaseline;
  //formerly *label1, *label2, *label3, *lblTable;
  QLabel *lblColumn, *lblFormat, *lblPrecision, *lblTable;

QFont m_labelFont, m_scaleFont;

  int m_mappedaxis;
  //m_mappedaxis is eqivelent to maptoQwtAxisId() as that should be passed into the constuctor via the enum; 

  QStringList tablesList;

  void setEnabled();

  void updateAxisType(int axis);
  void updateTitleBox(int axis);
  void updateShowBox(int axis);
  void updateAxisColor(int);
  void setTicksType(int);
  void setLabelsNumericFormat(int);
  void updateLabelsFormat(int);
  void setBaselineDist(int);
};
#endif /* AXISAXISDETAILS_H_ */