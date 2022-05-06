t <html><head><title>Tension de salida</title></head>
i pg_header.inc
t <h2 align=center><br>Tension de salida</h2>
t <p><font size="2">Esta pagina permiter ver los valores de la
t  tension de salida del amplificador de instrumentación<br>
t  This Form uses a <b>POST</b> method to send data to a Web server.</font></p>
t <form action=vo.cgi method=post name=cgi>
t <input type=hidden value="vo" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Valor</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>Valor de Vo en palabra digital</td>
c d 1 <td><b>%d (0x%03X)</b></td></tr>
t <tr><td><img src=pabb.gif>Valor de Vo en voltios</td>
c d 2 <td><b>%5.3f V</b></td></tr></font></table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Obtener Vo" id="sbm">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.
