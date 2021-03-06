#include	<wxMapPolygon.h>
#include	<wxMapObject.h>
#include    <string>
#include    <wx/tokenzr.h>
#include    <wx/log.h>
#include    <wx/intl.h>

wxMapPolygon::wxMapPolygon(std::vector<wxMapPoint> const& vPoints, float opacity, float weight, wxString color) :
    coordinates(vPoints),
    cOpacity(opacity),
    cWeight(weight),
    cColor(color)
   
{
    cType = EMapObjectType::POLYGON;
}

wxString wxMapPolygon::GetJavaScriptAdd(wxString map) const
{
    wxString js;
    for (int i=0; i<coordinates.size(); ++i)
    {
        js += wxString::Format("polygon_coord_add(%.6lf,%.6lf);\n", coordinates[i].x, coordinates[i].y);
    }
    js += "polygon_add(" + wxString::Format("%s,%f,%f,'", map, cOpacity,cWeight) + cColor + "' ); \n";
    return wxString(js);
}

pwxMapPolygon wxMapPolygon::Create(std::vector<wxMapPoint> const &vPoints, float opacity, float weight, wxString color)
{
    return boost::make_shared<wxMapPolygon>(vPoints,opacity,weight,color);
}

wxString wxMapPolygon::GetRemoveString(wxString const& map)
{
    return wxString::Format("polygon_remove(%d, %s); \n", cLeafletId, map);
}