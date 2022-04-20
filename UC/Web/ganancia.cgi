t <html><head><title>Ganancia</title></head>
i pg_header.inc
t <h2 align=center><br>Ganancia Control</h2>
t <p><font size="2">Esta pagina permiter ver y cambiar el valor de la ganancia.
t  Para modificar la ganancia selecciona valor de la lista<br>
t  This Form uses a <b>POST</b> method to send data to a Web server.</font></p>
t <form action=ganancia.cgi method=post name=cgi>
t <input type=hidden value="ganancia" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Valor</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
c a 1 <tr><td><img src=pabb.gif>Ganancia actual</td><td><b>%d</b></td></tr>
t <tr><td><img src=pabb.gif>Introducir ganancia</td>
t <td><select name="gain_sel" onchange="submit();">
c a 2 <option %s>1</option><option %s>5</option><option %s>10</option>
c a 3 <option %s>50</option><option %s>100</option></select></td></tr>
t </font></table></form>
i pg_footer.inc
. End of script must be closed with period.
