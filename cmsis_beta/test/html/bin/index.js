Module.addOnPostRun(() => {
  Module.Init ();
  Convert();
  document.getElementById('convert').disabled = false;
});
function Convert () {
  const tab = Module.Convert ();
  const per = document.getElementById('peripherals');
  per.innerHTML = tab;
  document.getElementById('registers').innerHTML = 'Registers';
}
function Register (order) {
  // console.log(order);
  const txt = Module.PrintPerif (order);
  const reg = document.getElementById('registers');
  reg.innerHTML = txt;
}
function RegisterDesc (p, r) {
  //console.log(p, r);
  const txt = Module.PrintReg (p, r);
  const reg = document.getElementById('fields');
  reg.innerHTML = txt;
}
function Field (per, reg, fie) {
  // console.log('per ' ,per, ',reg ', reg, ',fie ', fie);
  const txt = Module.PrintField (per, reg, fie);
  const fld = document.getElementById('fields');
  fld.innerHTML = txt;
}
/* Tato část přidává uživatelské soubory. */
function readFile(file) {
  const reader = new FileReader();
  reader.addEventListener('load', (event) => {
    const context = event.target.result;
    Module.setXml (context);
    const tab = Module.fromString ();
    const per = document.getElementById('peripherals');
    per.innerHTML = tab;
    const err = Module.PrintERR ();
    document.getElementById('fields').innerHTML = err;
    document.getElementById('registers').innerHTML = 'Registers';
  });
  reader.readAsText(file);
}
function OnClickUpl () {
  var element = document.createElement('input');
  element.setAttribute('type', 'file');
  element.setAttribute('accept', '.svd, .txt');
  element.style.display = 'none';
  element.addEventListener('change', (event) => {
    const fileList = event.target.files;
    for (n=0; n<fileList.length; n++) {
      const file = fileList[n];
      // console.log (file);
      readFile (file);
    }
  });
  document.body.appendChild(element);
  element.click();
  document.body.removeChild(element);
}
