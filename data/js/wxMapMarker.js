
// js code that is used by wxWebMap (executed with wxWebView::RunScript, which also receives returned values). See wxWebMap and wxMapMarker.
// The coded is added to any compatible HTML file using wxMapHtml.
var markers = new Array();
function marker_add(lat, lon, map) {
    var marker = new L.marker([lat, lon]); marker.addTo(map); 
    markers[marker._leaflet_id] = marker;
    return "MARKER," + marker._leaflet_id + "," + lat + "," + lon;
};
function marker_add_drag(lat, lon, map) {
    var marker = new L.marker([lat, lon], { draggable: true }); marker.addTo(map); 
    markers[marker._leaflet_id] = marker;
    return "MARKER," + marker._leaflet_id + "," + lat + "," + lon;
};
function mapobject_remove(id, map) {
    map.removeLayer(markers[id]);
    delete markers[id];
};
function image_add(lLat, lLon, rLat, rLon, map) {
    // Kvar att fixa:
    // 1. Kolla vad koordinaterna inneh�ller och varf�r de inte fungerar n�r man anv�nder dem som h�rn
    // 2. Fixa s� att det g�r att ladda upp en bild fr�n javaScript-funktionen (h�r) fr�n en filePath
    // 3. L�gg till pathen till wxMapImage s� att den kommer fr�n input
    // 4. G�r s� att alla bilder laddas upp (nu syns bara 1 av bilderna fr�n textfilen)
    imageUrl = "file://localhost/C:/Users/sandr/OneDrive/Desktop/DFG/Jyvaskyla/Jyvaskyla/images/004304-052412060557-CAM1.jpg";
    //url = imageUrl.replace(/\\/g, "/");
    var center = [lLat + (59.426180 - 59.326180), lLon + (18.472263 - 18.072263)]; //Calculation for bounds
    imageBounds = [center, [rLat, rLon]];
    var image = L.imageOverlay(imageUrl, imageBounds);
    image.addTo(map);
    return "IMAGE," + image._leaflet_id + "," + lLat + "," + lLon + "," + rLat + "," + rLon;

    //(59.426180 - 59.326180) (18.472263 - 18.072263)
};