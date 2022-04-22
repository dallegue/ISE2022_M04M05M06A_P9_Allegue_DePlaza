t <html><head><title>Overload</title></head>
i pg_header.inc
t <h2 align=center><br>Overload</h2>
t <p><font size="2">Esta pagina permiter ver y editar la info relativa al overload.
t  Para modificar el valor de overload selecciona valor de la lista.<br>
t  El estado de overload indica si la tension de salida ha superado el valor
t  de overload en el ultimo segundo<br>
t  This Form uses a <b>POST</b> method to send data to a Web server.</font></p>
t <form action=overload.cgi method=post name=cgi>
t <input type=hidden value="overload" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Valor</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
c b 1 <tr><td><img src=pabb.gif>Overload actual</td><td><b>%d</b></td></tr>
t <tr><td><img src=pabb.gif>Introducir overload</td>
t <td><select name="overload_sel" onchange="submit();">
c b 2 <option %s>1</option><option %s>2</option><option %s>3</option>
c b 3 <option %s>4</option><option %s>5</option><option %s>6</option>
c b 4 <option %s>7</option><option %s>8</option><option %s>9</option>
c b 5 <option %s>10</option></select></td></tr>
t <tr><td><img src=pabb.gif>Habilitar/deshabilitar interrupcion por overload</td>
c b 6 <td><input type=checkbox name=overload_int OnClick="submit();" %s></td></tr>
t <tr><td><img src=pabb.gif>Estado de overload</td>
c b 7 <td><input type="checkbox" id="overload_estado" %s disabled>
t </td></tr></font></table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Actualizar" id="sbm">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.
