const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("securewipeApi", {
  getVersion: () => ipcRenderer.invoke("securewipe:get-version"),
  getMethods: () => ipcRenderer.invoke("securewipe:get-methods"),
  getDisks: () => ipcRenderer.invoke("securewipe:get-disks"),
  startWipe: (payload) => ipcRenderer.invoke("securewipe:start-wipe", payload),
  getWipeStatus: (opId) => ipcRenderer.invoke("securewipe:get-wipe-status", opId),
  cancelWipe: (opId) => ipcRenderer.invoke("securewipe:cancel-wipe", opId),
  getCertificate: (opId) => ipcRenderer.invoke("securewipe:get-certificate", opId),
  getAuditHistory: () => ipcRenderer.invoke("securewipe:get-audit-history")
});
