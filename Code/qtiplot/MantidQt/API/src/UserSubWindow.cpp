//----------------------------------
// Includes
//----------------------------------
#include "MantidQtAPI/UserSubWindow.h"

#include <QIcon>
#include <QMessageBox>
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>

using namespace MantidQt::API;

//------------------------------------------------------
// Public member functions
//------------------------------------------------------
/**
 * Default Constructor
 */
UserSubWindow::UserSubWindow(QWidget* parent) :  QMdiSubWindow(parent), m_bIsInitialized(false), m_ifacename("")
{
}

/**
 * Destructor
 */
UserSubWindow::~UserSubWindow()
{
}

/**
 * Create the layout for this dialog.
 */
void UserSubWindow::initializeLayout()
{
  if( isInitialized() ) return;

  //Calls the derived class function
  this->initLayout();

  //Se the object name to the interface name
  setObjectName(m_ifacename);
  setOption(QMdiSubWindow::RubberBandResize);

  //Set the icon
  setWindowIcon(QIcon(":/MantidPlot_Icon_32offset.png"));

  
  m_bIsInitialized = true;
}

/**
 * Has this window been initialized yet
 *  @returns Whether initialzedLayout has been called yet
 */
bool UserSubWindow::isInitialized() const
{ 
  return m_bIsInitialized; 
}

//--------------------------------------
// Protected member functions
//-------------------------------------
/**
 * Raise a dialog box with some information for the user
 * @param message The message to show
 */

void UserSubWindow::showInformationBox(const QString & message)
{
  if( !message.isEmpty() )
  {
    QMessageBox::information(this, this->windowTitle(), message);
  }
}

/**
 * Execute a piece of Python code and the output that was written to stdout, i.e. the output from print
 * statements
 * @param code The code to execute
 * @param no_output An optional flag to specify that no output is needed. If running only small commands enable this
 * as it should be faster. The default value is false
 */
QString UserSubWindow::runPythonCode(QString & code, bool no_output)
{
  if( no_output )
  {
    emit runAsPythonScript(code);
    return QString();
  }
  
  // Otherwise we need to gather the information from stdout. This is achieved by redirecting the stdout stream
  // to a temproary file and then reading its contents
  // A QTemporaryFile object is used since the file is automatically deleted when the object goes out of scope
  QTemporaryFile tmp_file;
  if( !tmp_file.open() )
  {
    showInformationBox("An error occurred opening a temporary file in " + QDir::tempPath());
    return QString();
  }
  //The file name is only valid when the file is open
   QString tmpstring = tmp_file.fileName();
   tmp_file.close();
   code.prepend("import sys; sys.stdout = open('" + tmpstring + "', 'w')\n");

   //   showInformationBox(code);
   emit runAsPythonScript(code);

   //Now get the output
   tmp_file.open();
   QTextStream stream(&tmp_file);
   tmpstring.clear();
   while( !stream.atEnd() )
   {
     QString line = stream.readLine();
     //This bizarre line is here because in qtiplot I use a Python trace function
     //to print the current line number and then capture this and adjust the arrow in the
     //script window accordingly. Unfortunately this means lines such as MTDPYLN: # appear everywhere
     //in the redirected output
     if( line.contains("MTDPYLN") ) continue;
     tmpstring.append(line.trimmed() + "\n");
   }
   return tmpstring;
}

//--------------------------------------
// Private member functions
//-------------------------------------
/**
 * Set the interface name
 * @param iface_name The name of the interface
 */
void UserSubWindow::setInterfaceName(const QString & iface_name)
{
  m_ifacename = iface_name;
}
