// vselect.js — vertical (column) selection for PyTorch memory_viz.
//
// A vertical selection always takes the FULL vertical extent at the chosen time
// range: every block alive there, never a partial north-south slice. Selection is
// therefore purely along the time axis.
//
// Data source: each <polygon> the viz draws is d3-bound to one entry of
// `data.allocations_over_time`, shape {elem, timesteps[], offsets[], size, color},
// where `elem` indexes into the `elements` array (the "31 entries").
//
// Console API:
//   vsel.series()            -> the bound stack objects read back from the DOM
//   vsel.tmax                -> largest timestep
//   vsel.at(t)               -> full column alive at timestep t
//   vsel.range(t0, t1)       -> every block alive anywhere in [t0, t1]
//   vsel.enable()            -> drag on the plot to select; result in vsel.selection
//   vsel.disable()
//   vsel.selection           -> last selection (array)
//   vsel.csv(sel)            -> selection as CSV text

(function () {
  const vsel = {
    device: 0,
    selection: [],
    _svg: null,
    _overlay: null,
    _selectionOverlay: null,
    _highlightedPolygons: [],
    _handlers: null,
  };

  // ---- raw stack objects, read straight off the rendered polygons -----------
  function plotSvg() {
    let best = null;
    let bestCount = -1;
    for (const svg of document.querySelectorAll('svg')) {
      const n = svg.querySelectorAll('polygon').length;
      if (n > bestCount) {
        bestCount = n;
        best = svg;
      }
    }
    return best;
  }

  vsel.series = function () {
    const svg = plotSvg();
    if (!svg) return [];
    const out = [];
    for (const p of svg.querySelectorAll('polygon')) {
      const d = p.__data__;
      if (d && Array.isArray(d.timesteps) && d.timesteps.length) {
        out.push(d);
      }
    }
    return out;
  };

  Object.defineProperty(vsel, 'tmax', {
    get() {
      let m = 0;
      for (const s of vsel.series()) {
        const t = s.timesteps[s.timesteps.length - 1];
        if (t > m) m = t;
      }
      return m;
    },
  });

  // `elements` is only reachable if process_alloc_data.js was patched to return it
  // (see setup.sh). Without it we still report elem index / size / timesteps.
  function elementsArray() {
    if (globalThis.__vsel_elements) return globalThis.__vsel_elements;
    if (typeof globalThis.process_alloc_data === 'function' && globalThis.snapshot_cache) {
      const keys = Object.keys(globalThis.snapshot_cache);
      if (keys.length) {
        const snap = globalThis.snapshot_cache[keys[0]];
        const d = globalThis.process_alloc_data(snap, vsel.device, false, 15000, false);
        if (d && d.elements) {
          globalThis.__vsel_elements = d.elements;
          return d.elements;
        }
      }
    }
    return null;
  }

  function describe(s) {
    const ts = s.timesteps;
    const row = {
      elem: s.elem,
      size: Array.isArray(s.size) ? s.size[0] : s.size,
      t_begin: ts[0],
      t_end: ts[ts.length - 1],
      offset: s.offsets[0],
    };
    const els = elementsArray();
    if (els && typeof s.elem === 'number' && els[s.elem]) {
      const e = els[s.elem];
      row.addr = e.addr;
      row.stream = e.stream;
      row.action = e.action;
      row.max_allocated_mem = e.max_allocated_mem;
      row.frames = e.frames;
    }
    return row;
  }

  function ensureSelectionStyles() {
    if (document.getElementById('vsel-style')) return;
    const style = document.createElement('style');
    style.id = 'vsel-style';
    style.textContent = `
      .vsel-highlight {
        stroke: #ffcc00 !important;
        stroke-width: 3px !important;
        paint-order: stroke;
        vector-effect: non-scaling-stroke;
        filter: drop-shadow(0 0 2px rgba(255, 204, 0, 0.85));
      }
    `;
    document.head.appendChild(style);
  }

  function clearSelectionVisuals() {
    for (const p of vsel._highlightedPolygons || []) {
      p.classList.remove('vsel-highlight');
    }
    vsel._highlightedPolygons = [];
    if (vsel._selectionOverlay) {
      vsel._selectionOverlay.remove();
      vsel._selectionOverlay = null;
    }
  }

  function showSelectionVisuals(svg, t0, t1, rawSelection) {
    clearSelectionVisuals();
    ensureSelectionStyles();

    const rect = svg.getBoundingClientRect();
    const overlay = document.createElement('div');
    overlay.style.cssText =
      'position:fixed;pointer-events:none;background:rgba(255,204,0,.16);' +
      'border-left:2px solid #ffcc00;border-right:2px solid #ffcc00;display:block;z-index:9998';
    overlay.style.top = rect.top + 'px';
    overlay.style.height = rect.height + 'px';

    const cal = calibrate(svg);
    if (cal) {
      const x0 = cal.toClientX(t0);
      const x1 = cal.toClientX(t1);
      overlay.style.left = Math.min(x0, x1) + 'px';
      overlay.style.width = Math.max(3, Math.abs(x1 - x0)) + 'px';
    } else {
      overlay.style.left = '0px';
      overlay.style.width = '3px';
    }

    document.body.appendChild(overlay);
    vsel._selectionOverlay = overlay;

    for (const s of rawSelection || []) {
      for (const p of svg.querySelectorAll('polygon')) {
        if (p.__data__ === s) {
          p.classList.add('vsel-highlight');
          vsel._highlightedPolygons.push(p);
          break;
        }
      }
    }
  }

  // ---- the two selection primitives ---------------------------------------
  vsel.at = function (t, raw) {
    const hit = vsel.series().filter(s => {
      const ts = s.timesteps;
      return t >= ts[0] && t <= ts[ts.length - 1];
    });
    hit.sort((a, b) => a.offsets[0] - b.offsets[0]);   // bottom -> top
    vsel.selection = raw ? hit : hit.map(describe);
    const svg = plotSvg();
    if (svg) showSelectionVisuals(svg, t, t, hit);
    return vsel.selection;
  };

  vsel.range = function (t0, t1, raw) {
    const lo = Math.min(t0, t1);
    const hi = Math.max(t0, t1);
    const hit = vsel.series().filter(s => {
      const ts = s.timesteps;
      return ts[ts.length - 1] >= lo && ts[0] <= hi;
    });
    hit.sort((a, b) => a.offsets[0] - b.offsets[0]);
    vsel.selection = raw ? hit : hit.map(describe);
    const svg = plotSvg();
    if (svg) showSelectionVisuals(svg, lo, hi, hit);
    return vsel.selection;
  };

  vsel.total = function (sel) {
    const rows = sel || vsel.selection;
    let sum = 0;
    for (const r of rows) sum += (r.size || 0);
    return sum;
  };

  vsel.csv = function (sel) {
    const rows = sel || vsel.selection;
    if (!rows.length) return '';
    const cols = ['elem', 'size', 't_begin', 't_end', 'offset', 'addr'];
    const lines = [cols.join(',')];
    for (const r of rows) lines.push(cols.map(c => (r[c] === undefined ? '' : r[c])).join(','));
    return lines.join('\n');
  };

  // ---- pixel <-> timestep, calibrated from the drawn geometry --------------
  // Two (timestep, x) samples taken from real polygon points define the linear map,
  // so nothing depends on internal margins or on the zoom transform being identity.
  function calibrate(svg) {
    const samples = [];
    for (const p of svg.querySelectorAll('polygon')) {
      const d = p.__data__;
      if (!d || !Array.isArray(d.timesteps) || !d.timesteps.length) continue;
      const pts = (p.getAttribute('points') || '').trim().split(/\s+/);
      if (!pts.length) continue;
      const x0 = parseFloat(pts[0].split(',')[0]);
      if (!isFinite(x0)) continue;
      samples.push([d.timesteps[0], x0, p]);
      if (samples.length > 200) break;
    }
    let a = null;
    let b = null;
    for (const s of samples) {
      if (!a) { a = s; continue; }
      if (s[0] !== a[0]) { b = s; break; }
    }
    if (!a || !b) return null;
    const scale = (b[1] - a[1]) / (b[0] - a[0]);   // px per timestep, user units
    const node = a[2];
    return {
      toTimestep(clientX) {
        const ctm = node.getScreenCTM();
        const pt = node.ownerSVGElement.createSVGPoint();
        pt.x = clientX;
        pt.y = 0;
        const local = pt.matrixTransform(ctm.inverse());
        return a[0] + (local.x - a[1]) / scale;
      },
      toClientX(t) {
        const ctm = node.getScreenCTM();
        const pt = node.ownerSVGElement.createSVGPoint();
        pt.x = a[1] + (t - a[0]) * scale;
        pt.y = 0;
        return pt.matrixTransform(ctm).x;
      },
    };
  }

  vsel.timestepAtClientX = function (clientX) {
    const svg = plotSvg();
    if (!svg) return null;
    const cal = calibrate(svg);
    return cal ? cal.toTimestep(clientX) : null;
  };

  // ---- drag to select a column / column range ------------------------------
  vsel.enable = function (onSelect) {
    vsel.disable();
    const svg = plotSvg();
    if (!svg) {
      console.warn('[vsel] no plot svg found — load a snapshot first');
      return;
    }
    vsel._svg = svg;

    const box = document.createElement('div');
    box.style.cssText =
      'position:fixed;pointer-events:none;background:rgba(0,120,255,.18);' +
      'border-left:1px solid #06c;border-right:1px solid #06c;display:none;z-index:9999';
    document.body.appendChild(box);
    vsel._overlay = box;

    let downX = null;
    const rect = () => svg.getBoundingClientRect();

    function paint(x0, x1) {
      const r = rect();
      box.style.display = 'block';
      box.style.left = Math.min(x0, x1) + 'px';
      box.style.width = Math.abs(x1 - x0) + 'px';
      box.style.top = r.top + 'px';          // full vertical extent, always
      box.style.height = r.height + 'px';
    }

    const onDown = ev => { downX = ev.clientX; paint(downX, downX); };
    const onMove = ev => { if (downX !== null) paint(downX, ev.clientX); };
    const onUp = ev => {
      if (downX === null) return;
      const cal = calibrate(svg);
      const t0 = cal ? cal.toTimestep(downX) : 0;
      const t1 = cal ? cal.toTimestep(ev.clientX) : 0;
      const sel = Math.abs(ev.clientX - downX) < 3
        ? vsel.at(Math.round(t0))
        : vsel.range(Math.round(t0), Math.round(t1));
      downX = null;
      box.style.display = 'none';
      console.log(`[vsel] t=[${Math.round(t0)}, ${Math.round(t1)}]  blocks=${sel.length}  bytes=${vsel.total(sel)}`);
      console.table(sel);
      if (onSelect) onSelect(sel);
    };

    svg.addEventListener('mousedown', onDown);
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup', onUp);
    vsel._handlers = {onDown, onMove, onUp};
    console.log('[vsel] drag across the plot to select a full vertical column range.');
  };

  vsel.disable = function () {
    if (vsel._svg && vsel._handlers) {
      vsel._svg.removeEventListener('mousedown', vsel._handlers.onDown);
      window.removeEventListener('mousemove', vsel._handlers.onMove);
      window.removeEventListener('mouseup', vsel._handlers.onUp);
    }
    if (vsel._overlay) vsel._overlay.remove();
    clearSelectionVisuals();
    vsel._svg = null;
    vsel._overlay = null;
    vsel._handlers = null;
  };

  globalThis.vsel = vsel;
  console.log('[vsel] ready — vsel.at(t) / vsel.range(t0,t1) / vsel.enable()');
})();
