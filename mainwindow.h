#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// ----------------------------------------------------------------------------
//
// PolyScope
// authored by William Hinsberg
//
// Copyright (C) 2024 Columbia Hill Technical Consulting
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
//
// ----------------------------------------------------------------------------

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>

#include "chainlist.h"
#include "monomerlist.h"
#include "monomersequence.h"
#include "coloration.h"
#include "additiveclusterlist.h"
#include "additivelist.h"

namespace Ui
{
class MainWindow;
}

class ComboBoxDelegate;

typedef QList<AdditiveClusterList*> AdditiveClusterListList;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget* parent = nullptr );
    ~MainWindow();

    bool initialize( int argc, char* argv[] );
    void appendText( const char* s );
    void appendText( const QString& s );
    void updateStatus( int chain_num, int monomer_num );
    void calculateBoxSize();
    void drawChains();
    void setCurrentChainTargetLength( int length );
    void updateCurrentChainLength( int length );
    enum COLORATION coloration() const;
    MonomerList* monomerList() { return &monomer_type_list; }
    MonomerSequence* sequenceList() { return &sequence_list; }
    AdditiveList* additiveList() { return &additive_list; }
    const QString& fileName() const { return file_name; }
    const QString& versionTextStr() const { return version_text;}
    const char* versionText() const;
    const QString& outputFolder() const { return output_folder; }
    void setOutputFolder( const QString&  f )  { output_folder = f; }
    void reload() { reloadButtonClicked();}
    void refreshChains();

protected:
    void createPlot();
    void parametersToDefaultVaLues();
    void updateElapsedTime();
    QString runningTime();
    void loadModel();
    bool exportAsCSV( const QString& fileName );
    void initializeMonomerWidgets();
    void updateWindowTitle( const QString& filename = QString() );
    int setupAdditiveCalculation();
    void speciateSpecies();

private:
    Ui::MainWindow* ui;
    float density;
    float brush_density;
    float x_proportion;
    float y_proportion;
    int target_chain_length;
    int total_num_chains;
    int num_chains_created;
    QTimer timer;
    QElapsedTimer  elapsed_time;
    int scan_index;
    QAction* pauseAction;
    QAction* abortAction;
    QAction* stepAction;
    QAction* slowAction;
    QAction* fastAction;
    MonomerList monomer_type_list;
    MonomerSequence sequence_list;
    AdditiveList additive_list;
    bool has_point_cloud_calculated;
    QString file_name;
    QString version_text;
    QString model_folder;
    QString output_folder;

    void enableRunButtons( bool state );
    void makeConnections();
    void readInputs();
    bool parseParameters( int argc, char* argv[] );
    void initializeUserEntryFields();
    void startCalculation();

protected slots:
    void exportAsGridButtonClicked();
    void exportAsCsvButtonClicked();
    void importMonomerButtonClicked();
    void exitButtonClicked();
    void exposeButtonClicked();
    void abortButtonClicked();
    void pauseButtonClicked();
    void singleStepButtonClicked();
    void slowButtonClicked();
    void fastButtonClicked();
    void newButtonClicked();
    void brushRadioButtonClicked();
    void filmRadioButtonClicked();
    void unconstrainedRadioButtonClicked();
    void pointCloudRadioButtonClicked();
    void scanButtonClicked();
    void reverseScanButtonClicked();
    void resetScanButtonClicked();
    void endOfScanButtonClicked();
    void showMonomerCheckBoxClicked();
    void saveButtonClicked();
    void loadButtonClicked();
    void reloadButtonClicked();
    void onTimeout();
    void sequenceTypeComboBoxActivated( int index );
    void monomerAddButtonClicked();
    void monomerRemoveButtonClicked();
    void sequenceAddButtonClicked();
    void sequenceRemoveButtonClicked();
    void additiveAddButtonClicked();
    void additiveRemoveButtonClicked();
    void updateMonomerList();
    void colorationComboBoxChanged( int );
    void speciateButtonClicked();
    void deleteRemovedMonomerFromSequenceLists( QString name );
    void updateActivations();
    void clearAdditiveListList();
    void scanChainSliderValueChanged( int newValue );
    void calculateRgClicked();
};


extern MainWindow* appWindow();
extern ChainList& chainList();
extern AdditiveClusterListList& additiveListList();

#endif // MAINWINDOW_H
