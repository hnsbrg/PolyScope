#if defined(_MSC_VER) /* MSVC Compiler */
/* 'identifier' : truncation from 'type1' to 'type2' */
#pragma warning ( disable : 4305 )
#endif


#include "qwt3d_graphplot.h"

using namespace Qwt3D;

// Data class (private)


GraphPlot::GraphData::GraphData()
{
    datatype_p = Qwt3D::GRAPH;
    setHull( ParallelEpiped() );
}

GraphPlot::GraphData::~GraphData()
{
    setHull( ParallelEpiped() );
    nodes.clear();
    bonds.clear();
}

bool GraphPlot::GraphData::empty() const
{
    return nodes.empty();
}

// Data class end


GraphPlot::GraphPlot( QWidget* parent, const QOpenGLWidget* shareWidget )
    : Plot3D( parent, shareWidget )
{
    plotlets_p[0].data = ValuePtr<Data>( new GraphData );
}

void GraphPlot::createOpenGlData( const Plotlet& pl )
{
    if ( pl.appearance->plotStyle() == NOPLOT )
        return;

    const GraphData& data = dynamic_cast<const GraphData&>( *pl.data );

    //   glEnable( GL_POLYGON_SMOOTH );
    //   glEnable( GL_LINE_SMOOTH );

    Stick s( ( hull().maxVertex - hull().minVertex ).length() / 750, 16 );

    for ( unsigned i = 0; i != data.bonds.size(); ++i )
    {
        const Triple& beg = data.bonds[i].first;
        const Triple& end = data.bonds[i].second;
        s.setColor( data.bonds[i].col );

        if ( 0.0 < data.bonds[i].col.a )
        {
            s.draw( beg, end );
        }
    }


    Ball b( ( hull().maxVertex - hull().minVertex ).length() / 250, 8 );

    b.setColor( RGBA( 0.5, 0, 0 ) );

    for ( unsigned j = 0; j != data.nodes.size(); ++j )
    {
        b.setRadius( data.nodes[j].radius );
        b.setColor( data.nodes[j].col );

        if ( 0.0 < data.nodes[j].col.a )
        {
            b.draw( data.nodes[j].pos );
        }
    }
}


/*!
Convert user defined graph data to internal structure.
See also Qwt3D::TripleVector and Qwt3D::EdgeVector

\param append For append==true the new dataset will be appended. If false (default), all data  will
be replaced by the new data. This includes destruction of possible additional datasets/Plotlets.
\return Index of new entry in dataset array (append == true), 0 (append == false) or -1 for errors
*/
int GraphPlot::createDataset( AtomVector const& nodes, BondVector const& bonds, bool append /*= false*/ )
{

    int ret = prepareDatasetCreation<GraphData>( append );
    if ( ret < 0 )
        return -1;

    GraphData& data = dynamic_cast<GraphData&>( *plotlets_p[ret].data );

    data.nodes = nodes;
    data.bonds = bonds;
    data.setHull( Qwt3D::hull( nodes ) );
    updateData();
    createCoordinateSystem();

    return ret;
}

