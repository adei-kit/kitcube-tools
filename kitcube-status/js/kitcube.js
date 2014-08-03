function hex2rgb( colour ) {
    var r,g,b;
    if (colour.charAt(0) == '#') {
        colour = colour.substr(1);
    }

    r = "" + colour.charAt(0) + colour.charAt(1); //get color at 1 position
    g = "" + colour.charAt(2) + colour.charAt(3);
    b = "" + colour.charAt(4) + colour.charAt(5);

    r = parseInt( r, 16 ); //parse to int
    g = parseInt( g, 16 );
    b = parseInt( b , 16 );
    return new Array(r, g, b);
}

function dec2hex(i) {
    return (i+0x100).toString(16).substr(-2).toUpperCase();
}

/* Generate colors from begin color [r,g,b] array to end color [r,g,b] array */

function genColors(a, b, bands) {
    var myColors = [];
    var i;
    var delta = [];        // Difference between color in each channel
                           // Compute difference between each color
    for (i = 0; i < 3; i++){
        delta[i] = (a[i] - b[i]) / bands;
    }

                           // Use that difference to create your bands
    for (i = 0; i < bands; i++){
        var r = Math.round(a[0] - delta[0] * i);
        var g = Math.round(a[1] - delta[1] * i);
        var b = Math.round(a[2] - delta[2] * i);

        var finalColor = "#" + dec2hex(r) + dec2hex(g) + dec2hex(b);
        myColors.push(finalColor);
    }

    return myColors;
}

/* Buttons highlighting */
function switchHighlight() {
    $(this).siblings().each(function() {
        $(this).removeClass("highlight");
    }); 
    $(this).addClass("highlight");
}

function formatDate(date) {
    var dd = date.getDate()
    if ( dd < 10 ) dd = '0' + dd;
    var mm = date.getMonth()+1
    if ( mm < 10 ) mm = '0' + mm;
    var yy = date.getFullYear();
    if ( yy < 10 ) yy = '0' + yy;
    return dd+'.'+mm+'.'+yy;
}

function sensor(id, name, unit, unit2) {
    var e = document.createElement('li');
    var link = document.createElement('a');
    e.setAttribute('id', id);
    e.setAttribute('unit', unit);
    e.setAttribute('title', name);
    if (unit2 !== undefined)
        e.setAttribute('unit2', unit2);
    link.setAttribute('href', '#');
    link.innerHTML = name;
    e.appendChild(link);
    return e;
}

function updateData(script, id ,date) {
    $.ajax({
        url: 'scripts/'+script + '?id='+id + '&date='+date,
        method: 'post',
    });
}

function makePlotD1(event) {
    var panelname = $(this).parents('div .ui-widget-content').attr('id');
    var timekey = $('#' + panelname + ' input[type=radio]:checked').val();
    var id = $(this).attr('id');
    var title = $(this).attr('title');
    //console.log(title);
    var targetdiv = event.data.target;
    var targetdivwidth = parseFloat($(targetdiv).css('width'));
    var unit = $(this).attr('unit');
    
    var d = new Date();
    var stamp0 = new Date(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate(),0,0,0,0).getTime();
    //var stamp0 = new Date(2013, 4, 30, 0, 0, 0, 0).getTime();
    stamp0 = stamp0/1000 - d.getTimezoneOffset()*60;
    console.log(d.getUTCFullYear(), d.getUTCMonth()+1, d.getUTCDate(), stamp0);

    var stamp = stamp0;
    if (timekey === '2d')
        stamp = stamp0 - 2*86400;
    else if (timekey === '1d')
        stamp = stamp0 - 86400;

    $.ajax({
        url: 'cache/' + panelname + '.' + id + '.' + stamp + '.json',
        method: 'post',
        dataType: 'json',
        data: {timekey: timekey, id: id},
        success: plotdata
    });

    function plotdata(data) {
        var dataset = [];
        var rows = data['data'].length;

        var factor = Math.floor(data['time'].length/targetdivwidth/3 + 0.5);
        if (factor == 0) factor = 1;
        console.log(factor);

        for (var w = 0; w < rows; w++) {
          var temp = [];
          for (var i = 0; i < data['time'].length; i+=factor) {
              temp.push([data['time'][i]*1000, data['data'][w][i]]);
          }
          dataset.push(temp);
        }

        var d0 = new Date(data['time'][0]*1000);
        var dmin = Date.UTC(d0.getUTCFullYear(), d0.getUTCMonth(), d0.getUTCDate(), 0, 0, 0);
        var dmax = dmin + 86400000;
        var hour = 3600000;
        var ticks = [];
        for (var i=0; i<=24; i+=3) ticks.push(dmin + i*hour);

        var options = {
            canvas: true,
            xaxis: {
                mode: "time",
                ticks: ticks,
                min: dmin,
                max: dmax,
                timeformat: "%H:%M",
                axisLabelUseCanvas: true,
                axisLabelPadding: 16,
                axisLabelFontSizePixels: 16,
                axisLabelFontFamily: 'Arial',
                axisLabelColour: 'black',
                axisLabel: 'UTC Time   on   ' + formatDate(new Date(dmin)),
            },
            yaxis: {
                labelWidth: 40,
                axisLabelUseCanvas: true,
                axisLabelFontFamily: 'Arial',
                axisLabel: '[ ' + unit + ' ]'
            },
            series: {
                shadowSize: 0,
            },
        }

        var newplot = $.plot(targetdiv, dataset, options);

        /*
        var divholder = document.createElement('div');
        var divelement = document.createElement('div');
        divelement.style.width=600+'px';
        divelement.style.height=300+'px';
        var title = document.createElement('p');
        title.innerHTML = 'abc';
        divholder.appendChild(title);
        divholder.appendChild(divelement);
        var printversion = $.plot(divelement, dataset, options);
        console.log(printversion);
        */

        var canvas = newplot.getCanvas();
        var img = canvas.toDataURL("img/png");

        $('div #'+panelname).data('img1', img);
        $('div #'+panelname).data('title1', title);
    }
}

function makePlotD2() {
    var panelname = $(this).parents("div .ui-widget-content").attr('id');
    var time_choice = $('#' + panelname + ' input[type=radio]:checked').val();
    var unit = $(this).attr('unit');
    var unit2 = $(this).attr('unit2');
    var title = $(this).attr('title');

    $.ajax({
        url: "cache/" + panelname + "." +  $(this).attr('id') + "." + time_choice + ".json",
        method: "post",
        dataType: "json",
        data: {time_choice: time_choice, id: $(this).attr('id')},
        success: plotdata2
    });

    function plotdata2(data) {
        var d0 = new Date(data["xmin"]*1000);
        var dmin = Date.UTC(d0.getUTCFullYear(), d0.getUTCMonth(), d0.getUTCDate(), 0, 0, 0);
        var dmax = dmin + 86400000;
        var hour = 3600000;
        var ticks = [];
        for (var i=0; i<=24; i+=3) ticks.push(dmin + i*hour);

        var options_left = {
            canvas: true,
          series: {
              lines: {
                  lineWidth: 1
              },
              shadowSize: 0.5,
          },
          xaxis: {
              mode: 'time',
              min: dmin,
              max: dmax,
              ticks: ticks,
              timeformat: "%H:%M",
              axisLabelUseCanvas: true,
              axisLabelPadding: 16,
              axisLabelFontSizePixels: 16,
              axisLabelFontFamily: 'Arial',
              axisLabelColour: 'black',
              axisLabel: 'UTC Time   on   ' + formatDate(new Date(dmin)),
          },
          yaxis: {
              min: data["ymin"],
              max: data["ymax"],
              labelWidth: 40,
              axisLabelUseCanvas: true,
              axisLabelFontFamily: 'Arial',
              axisLabel: '[ ' + unit + ' ]'
          },
          grid: {
              margin: {}
          },
          hooks: {
              drawSeries: [fillShape]
          }
        }

    var dataset=[];

    for (var i = 0; i < data['data'].length; i++) {
        for (var j = 0; j < data['data'][i]["path"].length; j++) {
            if (data['data'][i]["path"][j] != null)
                data['data'][i]["path"][j][0] *= 1000;
        }
        dataset.push({
            data: data['data'][i]["path"],
            color: data['data'][i]["color"]});
    }

    function fillShape(plot, ctx, series) {

        var plotOffset = plot.getPlotOffset();
        var offset_x = plotOffset.left;
        var offset_y = plotOffset.top;

        function coord_x(x) {
            return offset_x + series.xaxis.p2c(x)
        };
        function coord_y(y) {
            return offset_y + series.yaxis.p2c(y)
        };

        var data = series.data;
        var chk = 0;

        ctx.beginPath();
        for (var i = 0; i < data.length; i++) {
            if (data[i] == null) {
                chk = 0;
                continue;
            }
            if (chk == 0) {
                ctx.moveTo(coord_x(data[i][0]), coord_y(data[i][1]));
                chk = 1;
            } else {
                ctx.lineTo(coord_x(data[i][0]), coord_y(data[i][1]));
            }
        }
        ctx.fillStyle = series.color;
        ctx.fill();
        ctx.closePath();
    }

    var colorbar = [];
    var colorbar_val = [];
    var color = '';
    var value = -1000;
    for (var i=0; i<data['data'].length; i++) {
        var newcolor = data['data'][i]['color'];
        var newval = data['data'][i]['layer'];
        if (newval < value) {
            break;
        } else {
            value = newval;
        }
        if (newcolor !== color) {
            colorbar.push(newcolor);
            colorbar_val.push(newval);
            color = newcolor;
        }
    }

    var ymin = colorbar_val[0];
    var ymax = colorbar_val[colorbar_val.length-1];

    console.log(colorbar, colorbar_val);

    var cntrArr = []; //last points
    var tempObj = {
        data: [],
        color: "",
    };

    var cntDataset = [];
    for (var i=0; i<data['data'].length; i++) {
        cntDataset.push({
            data: [[0,colorbar_val[i]],
                   [1,colorbar_val[i]],
                   [2,colorbar_val[i]]],
            color: colorbar[i]
        });
    }

        var options_right = {
            canvas: true,
          series: {
              lines:{
                  fillColor: {colors: colorbar},
                  fill: true,
              },
              shadowSize: 0,
          },
          xaxis: {
              axisLabel:  unit2,
              axisLabelPadding: 16,
              tickLength: 0,
              ticks: [0, 1],
              tickColor: 'transparent',
              tickFormatter: function(val, axis) {
                  return ""
              },
              axisLabelUseCanvas: true,
              axisLabelFontSizePixels: 12,
              axisLabelFontFamily: 'Arial',
          },
          yaxis: {
              min: ymin,
              max: ymax,
              position: "right",
              axisLabelUseCanvas: true,
              ticks: 10
          },
          grid: {
              margin: {left:0, right:0, bottom: 17}
          },
          hooks: {
          }
        }

        console.log('colorbar', cntDataset);
        console.log(cntDataset.length);

        var plot1 = $.plot("#plot2d", dataset, options_left);
        var plot2 = $.plot("#plotColorBar", cntDataset, options_right);

        var canvas1 = plot1.getCanvas();
        var canvas2 = plot2.getCanvas();

        var canvas3 = document.createElement('canvas');
        canvas3.width = 800;
        canvas3.height = 400;
        var ctx3 = canvas3.getContext('2d');

        ctx3.drawImage(canvas1, 0, 10);
        ctx3.drawImage(canvas2, 700, 10);

        var img = canvas3.toDataURL("img/png");
        $('div #'+panelname).data('img2', img);
        $('div #'+panelname).data('title2', title);
  }
}

