t <html><head><title>Flash</title></head>
i pg_header.inc
t <h2 align=center><br>Flash</h2>
t <p><font size="2">Esta pagina permiter ver los valores de timestamp
t  almacenados en la flash para ganancia y overload.<br>
t  This Form uses a <b>POST</b> method to send data to a Web server.</font></p>
t <form action=flash.cgi method=post name=cgi>
t <input type=hidden value="flash" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Valor</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>Timestamp almacenado en la flash</td>
t <td><select name="flash_sel" onchange="submit();">
c c 1 <option %s>Ganancia</option><option %s>Overload</option></select></td></tr>
t <tr><td><img src=pabb.gif>Valor</td>
c c 2 <td><b>%s</b></td></tr></font></table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Actualizar" id="sbm">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.
