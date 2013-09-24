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

#ifndef SCALEAXISDETAILS_H_
#define SCALEAXISDETAILS_H_

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

class ScaleAxisDetails: public QWidget
{
  Q_OBJECT
    //details for each axis in the Scale Tab
public:
  ScaleAxisDetails(ApplicationWindow* app, Graph* graph, int mappedaxis, QWidget *parent = 0); // populate and fill in with existing data
  virtual ~ScaleAxisDetails();
  void initWidgets();
  bool modified();
  void apply();
  bool valid();

private slots:
  //void updateMinorTicksList(int scaleType);
  //void endvalueChanged(double);
  //void startvalueChanged(double);
  void radiosSwitched();
  void setModified();

private:
  bool m_modified, m_initialised, validate();
  ApplicationWindow* d_app;
  Graph* d_graph;

  //formerly  *boxEnd, *boxStart, *boxStep, *boxBreakStart, *boxBreakEnd, *boxStepBeforeBreak, *boxStepAfterBreak;
  DoubleSpinBox *dspnEnd, *dspnStart, *dspnStep, *dspnBreakStart, *dspnBreakEnd, *dspnStepBeforeBreak, *dspnStepAfterBreak;
  //formerly *btnInvert, *boxLog10AfterBreak, *boxBreakDecoration;
  QCheckBox *chkInvert, *chkLog10AfterBreak, *chkBreakDecoration;
  //formerly *btnStep,*btnMajor
  QRadioButton *radStep, *radMajor;
  //formerly *boxMajorValue, *boxBreakPosition, *boxBreakWidth;
  QSpinBox *spnMajorValue, *spnBreakPosition, *spnBreakWidth;
  //formerly *boxAxesBreaks;
  QGroupBox *grpAxesBreaks;
  //formerly *boxMinorTicksBeforeBreak, *boxMinorTicksAfterBreak, *boxScaleType, *boxMinorValue, *boxUnit;
  QComboBox *cmbMinorTicksBeforeBreak, *cmbMinorTicksAfterBreak, *cmbScaleType, *cmbMinorValue, *cmbUnit;
  //formerly *boxScaleTypeLabel, *minorBoxLabel;
  QLabel *lblScaleTypeLabel, *lblMinorBox;
  //formerly *boxStartDateTime, *boxEndDateTime;
  QDateTimeEdit *dteStartDateTime, *dteEndDateTime;
  //formerly *boxStartTime, *boxEndTime;
  QTimeEdit *timStartTime, *timEndTime;

  int m_mappedaxis;
  void checkstep();
};

#endif /* SCALEAXISDETAILS_H_ */
