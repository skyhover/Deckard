/*******************************************************************************
 * Copyright (c) 2004, 2006 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/

package org.eclipse.debug.ui.memory;

import java.math.BigInteger;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.debug.core.DebugException;
import org.eclipse.debug.core.model.IMemoryBlock;
import org.eclipse.debug.core.model.IMemoryBlockExtension;
import org.eclipse.debug.core.model.MemoryByte;
import org.eclipse.debug.internal.ui.DebugUIMessages;
import org.eclipse.debug.internal.ui.DebugUIPlugin;
import org.eclipse.debug.internal.ui.IInternalDebugUIConstants;
import org.eclipse.debug.internal.ui.memory.IMemoryBlockConnection;
import org.eclipse.debug.internal.ui.memory.IPersistableDebugElement;
import org.eclipse.debug.internal.ui.preferences.IDebugPreferenceConstants;
import org.eclipse.debug.internal.ui.views.memory.MemoryViewUtil;
import org.eclipse.debug.internal.ui.views.memory.renderings.CopyTableRenderingToClipboardAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.FormatTableRenderingAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.FormatTableRenderingDialog;
import org.eclipse.debug.internal.ui.views.memory.renderings.GoToAddressAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.AbstractBaseTableRendering;
import org.eclipse.debug.internal.ui.views.memory.renderings.PrintTableRenderingAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.ReformatAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.ResetToBaseAddressAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingCellModifier;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingContentInput;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingContentProvider;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingLabelProvider;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingLabelProviderEx;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingLine;
import org.eclipse.debug.ui.DebugUITools;
import org.eclipse.debug.ui.IDebugUIConstants;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.Separator;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.resource.JFaceResources;
import org.eclipse.jface.text.Document;
import org.eclipse.jface.text.TextViewer;
import org.eclipse.jface.util.IPropertyChangeListener;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.IBasicPropertyConstants;
import org.eclipse.jface.viewers.ICellModifier;
import org.eclipse.jface.viewers.IColorProvider;
import org.eclipse.jface.viewers.IFontProvider;
import org.eclipse.jface.viewers.ILabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.custom.TableCursor;
import org.eclipse.swt.custom.TableEditor;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.FocusAdapter;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseTrackAdapter;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.TraverseEvent;
import org.eclipse.swt.events.TraverseListener;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.dialogs.PropertyDialogAction;
import org.eclipse.ui.model.IWorkbenchAdapter;
import org.eclipse.ui.part.PageBook;

/**
 * Abstract implementation of a table rendering.
 * <p>
 * Clients should subclass from this class if they wish to provide a
 * table rendering.
 * </p>
 * <p>
 *
 * The label of the rendering is constructed by retrieving the expression from
 * <code>IMemoryBlockExtension</code>.  For IMemoryBlock, the label is constructed
 * using the memory block's start address.
 * 
 * This rendering manages the change states of its memory bytes if the memory
 * block does not opt to manage the change states.  For IMemoryBlockExtension, if
 * the memory block returns false when #supportsChangeManagement() is called, this
 * rendering will calculate the change state for each byte when its content is updated.
 * Clients may manages the change states of its memory block by returning true when
 * #supportsChangeManagement() is called.  This will cause this rendering to stop
 * calculating the change states of the memory block.  Instead it would rely on the
 * attributes returned in the MemoryByte array to determine if a byte has changed.
 * For IMemoryBlock, this rendering will manage the change states its content.   
 * 
 *  When firing change event, be aware of the following:
 *  - whenever a change event is fired, the content provider for Memory View
 *    view checks to see if memory has actually changed.  
 *  - If memory has actually changed, a refresh will commence.  Changes to the memory block
 *    will be computed and will be shown with the delta icons.
 *  - If memory has not changed, content will not be refreshed.  However, previous delta information 
 * 	  will be erased.  The screen will be refreshed to show that no memory has been changed.  (All
 *    delta icons will be removed.)
 *    
 * Please note that these APIs will be called multiple times by the Memory View.
 * To improve performance, debug adapters need to cache the content of its memory block and only
 * retrieve updated data when necessary.
 * </p>

 * @since 3.1
 */
public abstract class AbstractTableRendering extends AbstractBaseTableRendering implements IPropertyChangeListener, IResettableMemoryRendering{	

	/**
	 *  Property identifier for the selected address in a table rendering
	 *  This property is used for synchronization between renderings.
	 */
	public static final String PROPERTY_SELECTED_ADDRESS = "selectedAddress"; //$NON-NLS-1$
	
	/**
	 * Property identifier for the column size in a table rendering
	 * This property is used for synchronization between renderings.
	 */
	public static final String PROPERTY_COL_SIZE = "columnSize"; //$NON-NLS-1$
	
	/**
	 * Property identifier for the top row address in a table rendering. 
	 * This property is used for synchronization between renderings.
	 */
	public static final String PROPERTY_TOP_ADDRESS = "topAddress"; //$NON-NLS-1$
	
	/**
	 * Property identifier for the row size in a table rendering
	 * This property is used for synchronization between renderings.
	 * @since 3.2
	 */
	public static final String PROPERTY_ROW_SIZE = "rowSize"; //$NON-NLS-1$
	
	private PageBook fPageBook;
	private TableViewer fTableViewer;
	private TextViewer fTextViewer;
	
	private int fBytePerLine;								// number of bytes per line: 16
	private int fColumnSize;								// number of bytes per column:  1,2,4,8
	private int fAddressableSize;	
	
	private boolean fIsShowingErrorPage;
	
	private TableRenderingContentProvider fContentProvider;
	private BigInteger fSelectedAddress;
	private TableRenderingContentInput fContentInput;
	private TableRenderingCellModifier fCellModifier;
	private boolean fIsCreated;
	private CellEditor[] fEditors;
	private String fLabel;
	private TableCursor fTableCursor;
	private boolean fIsDisposed;
	private TraverseListener fCursorTraverseListener;
	private KeyAdapter fCursorKeyAdapter;
	private BigInteger fTopRowAddress;
	
	private CopyTableRenderingToClipboardAction fCopyToClipboardAction;
	private GoToAddressAction fGoToAddressAction;
	private ResetToBaseAddressAction fResetMemoryBlockAction;
	private PrintTableRenderingAction fPrintViewTabAction;
	private ReformatAction fReformatAction;
	private ToggleAddressColumnAction fToggleAddressColumnAction;
	private EventHandleLock fEvtHandleLock = new EventHandleLock();
	private TableEditor fCursorEditor;
	private FocusAdapter fEditorFocusListener;
	private MouseAdapter fCursorMouseListener;
	private KeyAdapter fEditorKeyListener;
	private SelectionAdapter fCursorSelectionListener;
	private IWorkbenchAdapter fWorkbenchAdapter;
	private IMemoryBlockConnection fConnection;
	
	private boolean fIsShowAddressColumn = true;
	private SelectionAdapter fScrollbarSelectionListener;

	private PropertyDialogAction fPropertiesAction;
	
	private int fPageSize;
	private NextPageAction fNextAction;
	private PrevPageAction fPrevAction;

	private Shell fToolTipShell;
	private FormatTableRenderingAction fFormatRenderingAction;

	private IMenuListener fMenuListener;
	
	private class EventHandleLock
	{
		Object fOwner;
		
		public boolean acquireLock(Object client)
		{
			if (fOwner == null)
			{
				fOwner = client;
				return true;
			}
			return false;
		}
		
		public boolean releaseLock(Object client)
		{
			if (fOwner == client)
			{
				fOwner = null;
				return true;
			}
			return false;
		}
		
		public boolean isLocked()
		{
			return (fOwner != null);
		}
	}	
	
	
	private class ToggleAddressColumnAction extends Action {

		public ToggleAddressColumnAction() {
			super();
			PlatformUI.getWorkbench().getHelpSystem().setHelp(this, IDebugUIConstants.PLUGIN_ID
					+ ".ShowAddressColumnAction_context"); //$NON-NLS-1$
			updateActionLabel();
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see org.eclipse.jface.action.IAction#run()
		 */
		public void run() {
			fIsShowAddressColumn = !fIsShowAddressColumn;
			resizeColumnsToPreferredSize();
			updateActionLabel();
		}

		/**
		 * 
		 */
		private void updateActionLabel() {
			if (fIsShowAddressColumn) {
				setText(DebugUIMessages.ShowAddressColumnAction_0); 
			} else {
				setText(DebugUIMessages.ShowAddressColumnAction_1); 
			}
		}
	}
	
	
	private class NextPageAction extends Action
	{
		private NextPageAction()
		{
			super();
			setText(DebugUIMessages.AbstractTableRendering_4);
			PlatformUI.getWorkbench().getHelpSystem().setHelp(this, IDebugUIConstants.PLUGIN_ID + ".NextPageAction_context"); //$NON-NLS-1$ 
		}

		public void run() {
			BigInteger address = fContentInput.getLoadAddress();
			address = address.add(BigInteger.valueOf(getPageSizeInUnits()));
			handlePageStartAddressChanged(address);
		}
	}
	
	private class PrevPageAction extends Action
	{
		private PrevPageAction()
		{
			super();
			setText(DebugUIMessages.AbstractTableRendering_6);
			PlatformUI.getWorkbench().getHelpSystem().setHelp(this, IDebugUIConstants.PLUGIN_ID + ".PrevPageAction_context"); //$NON-NLS-1$
		}

		public void run() {
			BigInteger address = fContentInput.getLoadAddress();
			address = address.subtract(BigInteger.valueOf(getPageSizeInUnits()));
			handlePageStartAddressChanged(address);
		}
	}
	
	/**
	 * Constructs a new table rendering of the specified type.
	 * 
	 * @param renderingId memory rendering type identifier
	 */
	public AbstractTableRendering(String renderingId) {
		super(renderingId);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.jface.util.IPropertyChangeListener#propertyChange(org.eclipse.jface.util.PropertyChangeEvent)
	 */
	public void propertyChange(PropertyChangeEvent event) {
		// if memory view table font has changed
		if (event.getProperty().equals(IInternalDebugUIConstants.FONT_NAME))
		{
			if (!fIsDisposed)
			{			
				Font memoryViewFont = JFaceResources.getFont(IInternalDebugUIConstants.FONT_NAME);
				setFont(memoryViewFont);		
			}
			return;
		}
		
		if (event.getProperty().equals(IDebugUIConstants.PREF_PADDED_STR) ||
			event.getProperty().equals(IDebugUIConstants.PREF_MEMORY_HISTORY_KNOWN_COLOR) ||
			event.getProperty().equals(IDebugUIConstants.PREF_MEMORY_HISTORY_UNKNOWN_COLOR))
		{
			if (!fIsDisposed)
			{
				fTableViewer.refresh();
				fTableCursor.redraw();
			}
			return;
		}
		
		Object evtSrc = event.getSource();
		
		if (event.getProperty().equals(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE)) {
			// always update page size, only refresh if the table is visible
			getPageSizeFromPreference();
		}
		
		// do not handle event if the rendering is displaying an error
		if (isDisplayingError())
			return;
		
		// do not handle property change event if the rendering is not visible
		if (!isVisible())
			return;
		
		if (event.getProperty().equals(IDebugPreferenceConstants.PREF_DYNAMIC_LOAD_MEM)) {
			handleDyanicLoadChanged();
			return;
		}
		
		if (event.getProperty().equals(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE)) {
			if (!isDynamicLoad())
			{
				// only refresh if in non-autoload mode
				refresh();
			}
			return;
		}
		
		if (evtSrc == this)
			return;
		
		if (!(evtSrc instanceof IMemoryRendering))
			return;
		
		IMemoryRendering rendering = (IMemoryRendering)evtSrc;
		IMemoryBlock memoryBlock = rendering.getMemoryBlock();
		
		// do not handle event from renderings displaying other memory blocks
		if (memoryBlock != getMemoryBlock())
			return;
	
		String propertyName = event.getProperty();
		Object value = event.getNewValue();
		
		if (propertyName.equals(AbstractTableRendering.PROPERTY_SELECTED_ADDRESS) && value instanceof BigInteger)
		{
			selectedAddressChanged((BigInteger)value);
		}
		else if (propertyName.equals(AbstractTableRendering.PROPERTY_COL_SIZE) && value instanceof Integer)
		{
			columnSizeChanged(((Integer)value).intValue());
		}
		else if (propertyName.equals(AbstractTableRendering.PROPERTY_ROW_SIZE) && value instanceof Integer)
		{
			rowSizeChanged(((Integer)value).intValue());
		}
		else if (propertyName.equals(AbstractTableRendering.PROPERTY_TOP_ADDRESS) && value instanceof BigInteger)
		{
			if (needMoreLines())
			{
				if (isDynamicLoad())
					reloadTable(getTopVisibleAddress(), false);
			}
			topVisibleAddressChanged((BigInteger)value, false);
		}
		else if (propertyName.equals(IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS) && value instanceof BigInteger)
		{
			handlePageStartAddressChanged((BigInteger)value);
		}
	}

	private void handleDyanicLoadChanged() {
		
		// if currently in dynamic load mode, update page
		// start address
		updateSyncPageStartAddress();
		
		updateDynamicLoadProperty();
		if (isDynamicLoad())
		{
			refresh();
		}
		else
		{
			BigInteger pageStart = (BigInteger)getSynchronizedProperty(IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS);
			if (pageStart == null)
				pageStart = fTopRowAddress;
			handlePageStartAddressChanged(pageStart);
		}
	}

	private void updateDynamicLoadProperty() {
		
		boolean value = DebugUIPlugin
				.getDefault()
				.getPreferenceStore()
				.getBoolean(IDebugPreferenceConstants.PREF_DYNAMIC_LOAD_MEM);
		
		if (value != isDynamicLoad())
		{
			fContentProvider.setDynamicLoad(value);
		
			if (!fIsDisposed) {
				if (isDynamicLoad()) {
					fContentInput.setPostBuffer(20);
					fContentInput.setPreBuffer(20);
					fContentInput.setDefaultBufferSize(20);
					fContentInput.setNumLines(getNumberOfVisibleLines());
	
				} else {
					fContentInput.setPostBuffer(0);
					fContentInput.setPreBuffer(0);
					fContentInput.setDefaultBufferSize(0);
					fContentInput.setNumLines(fPageSize);
				}	
			}
		}
	}

	/**
	 * Handle top visible address change event from synchronizer
	 * @param address
	 */
	private void topVisibleAddressChanged(final BigInteger address, boolean force)
	{
		// do not handle event if rendering is not visible
		// continue to handle event if caller decides to force the rendering
		// to move to the top visible address even when the rendering
		// is not visible
		if (!isVisible() && !force)
			return;
		
		// do not handle event if the base address of the memory
		// block has changed, wait for debug event to update to
		// new location
		if (isBaseAddressChanged())
			return;
	
		if (!address.equals(fTopRowAddress))
		{
			fTopRowAddress = address;
			updateSyncTopAddress();
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
			{
			
				handleTopAddressChangedforExtended(address);
			}
			else
			{
				handleTopAddressChangedForSimple(address);
			}
		}
	}

	/**
	 * @param address
	 */
	private void handleTopAddressChangedForSimple(final BigInteger address) {
		// IMemoryBlock support
		int index = findAddressIndex(address);
		Table table = fTableViewer.getTable();
		if (index >= 0)
		{
			setTopIndex(table,  index);
		}
		
		if (isAddressVisible(fSelectedAddress))
			fTableCursor.setVisible(true);
		else
			fTableCursor.setVisible(false);
		
	}

	/**
	 * @param address
	 */
	private void handleTopAddressChangedforExtended(final BigInteger address) {
		
		Object evtLockClient = new Object();
		try 
		{
		if (!fEvtHandleLock.acquireLock(evtLockClient))
			return;
		
		if (!isAddressOutOfRange(address))
		{
			Table table = fTableViewer.getTable();
			int index = findAddressIndex(address);
			if (index >= 3 && table.getItemCount() - (index+getNumberOfVisibleLines()) >= 3)
			{
				// update cursor position
				setTopIndex(table, index);
			}
			else
			{
				int numInBuffer = table.getItemCount();
				if (index < 3)
				{
					if(isAtTopLimit())
					{
						setTopIndex(table, index);
					}
					else
					{
						if (isDynamicLoad())
							reloadTable(address, false);
						else
							setTopIndex(table, index);
					}
				}
				else if ((numInBuffer-(index+getNumberOfVisibleLines())) < 3)
				{
					if (!isAtBottomLimit() && isDynamicLoad())
						reloadTable(address, false);
					else
						setTopIndex(table, index);
				}
			}
		}
		else
		{	
			// approaching limit, reload table
			reloadTable(address, false);
		}
		
		if (isAddressVisible(fSelectedAddress))
			fTableCursor.setVisible(true);
		else
			fTableCursor.setVisible(false);
		}
		finally
		{
			fEvtHandleLock.releaseLock(evtLockClient);
		}
	}	
	
	/**
	 * @param value
	 */
	private void selectedAddressChanged(BigInteger value) {
		
		// do not handle event if the base address of the memory
		// block has changed, wait for debug event to update to
		// new location
		if (isBaseAddressChanged())
			return;
		
		try {
			// do not handle event if the event is out of range and the 
			// rendering is in non-dynamic-load mode, otherwise, will
			// cause rendering to continue to scroll when it shouldn't
			if (isDynamicLoad())
				goToAddress(value);
			else if (!isAddressOutOfRange(value))
				goToAddress(value);
		} catch (DebugException e) {
			// do nothing
		}
	}
	
	private void handlePageStartAddressChanged(BigInteger address)
	{
		// do not handle if in dynamic mode
		if (isDynamicLoad())
			return;
		
		if (fContentInput == null)
			return;
		
		if (!(getMemoryBlock() instanceof IMemoryBlockExtension))
			return;
		
		// do not handle event if the base address of the memory
		// block has changed, wait for debug event to update to
		// new location
		if (isBaseAddressChanged())
			return;
		
		if(fContentProvider.getBufferTopAddress().equals(address))
			return;
	
		BigInteger start = fContentInput.getStartAddress();
		BigInteger end = fContentInput.getEndAddress();
		
		// smaller than start address, load at start address
		if (address.compareTo(start) < 0)
		{
			if (isAtTopLimit())
				return;
			
			address = start;
		}
		
		// bigger than end address, no need to load, already at top
		if (address.compareTo(end) > 0)
		{
			if (isAtBottomLimit())
				return;
			
			address = end.subtract(BigInteger.valueOf(getPageSizeInUnits()));
		}
		
		fContentInput.setLoadAddress(address);
		refresh();
		updateSyncPageStartAddress();
		setTopIndex(fTableViewer.getTable(), 0);
		fTopRowAddress = address;
		updateSyncTopAddress();
		
		BigInteger selectedAddress = (BigInteger)getSynchronizedProperty(AbstractTableRendering.PROPERTY_SELECTED_ADDRESS);
		if (selectedAddress != null)
		{
			fSelectedAddress = selectedAddress;
			if (!isAddressOutOfRange(fSelectedAddress))
			{
				setCursorAtAddress(fSelectedAddress);
				fTableCursor.setVisible(true);
			}
			else
			{
				fTableCursor.setVisible(false);
			}
		}
	}

	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#createControl(org.eclipse.swt.widgets.Composite)
	 */
	public Control createControl(Composite parent) {
		
		fPageBook = new PageBook(parent, SWT.NONE);
		createErrorPage(fPageBook);
		createTableViewer(fPageBook);
		
		fTableViewer.getTable().redraw();
		createToolTip();
		
		return fPageBook;
	}

	/**
	 * Create the table viewer and other support controls
	 * for this rendering.
	 * 
	 * @param parent parent composite
	 */
	private void createTableViewer(Composite parent) {
		
		fTableViewer= new TableViewer(parent, SWT.SINGLE | SWT.H_SCROLL | SWT.V_SCROLL | SWT.HIDE_SELECTION | SWT.BORDER);
		
		TableRenderingLabelProvider labelProvider;
		if (hasCustomizedDecorations())
			labelProvider = new TableRenderingLabelProviderEx(this);
		else
			labelProvider = new TableRenderingLabelProvider(this);
		
		fTableViewer.setLabelProvider(labelProvider);
		
		fContentProvider = new TableRenderingContentProvider();
		fContentProvider.setDynamicLoad(DebugUIPlugin.getDefault().getPreferenceStore().getBoolean(IDebugPreferenceConstants.PREF_DYNAMIC_LOAD_MEM));
		
		fTableViewer.setContentProvider(fContentProvider);		
		fContentProvider.setViewer(fTableViewer);
		
		ScrollBar scroll = ((Table)fTableViewer.getControl()).getVerticalBar();
		scroll.setMinimum(-100);
		scroll.setMaximum(200);		

		fTableViewer.getTable().setHeaderVisible(true);
		fTableViewer.getTable().setLinesVisible(true);
		

		// set up addressable size and figure out number of bytes required per line
		fAddressableSize = -1;
		try {
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
				fAddressableSize = ((IMemoryBlockExtension)getMemoryBlock()).getAddressableSize();
		} catch (DebugException e1) {
			// log error and default to 1
			fAddressableSize = 1;
			displayError(e1);
			return;
			
		}
		if (getAddressableSize() < 1)
			fAddressableSize = 1;
		
// set up initial format
		setupInitialFormat();
		
// set up selected address		
		setupSelectedAddress();
		
		// figure out top visible address
		BigInteger topVisibleAddress = getInitialTopVisibleAddress();
		
		getPageSizeFromPreference();
		
		if (isDynamicLoad())
			fContentInput = new TableRenderingContentInput(this, 20, 20, 20, topVisibleAddress, getNumberOfVisibleLines(), false, null);
		else
		{
			BigInteger addressToLoad = topVisibleAddress;
			
			// check synchronization service to see if we need to sync with another rendering
			Object obj = getSynchronizedProperty(IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS);
			if (obj != null && obj instanceof BigInteger)
			{
				addressToLoad = (BigInteger)obj;
			}
			fContentInput = new TableRenderingContentInput(this, 0, 0, 0, addressToLoad, fPageSize, false, null);
		}
		
		fTableViewer.setInput(fContentInput);
		
		// set up cell modifier
		fCellModifier = new TableRenderingCellModifier(this);
		fTableViewer.setCellModifier(fCellModifier);
		
		// SET UP FONT		
		// set to a non-proportional font
		fTableViewer.getTable().setFont(JFaceResources.getFont(IInternalDebugUIConstants.FONT_NAME));
		if (!(getMemoryBlock() instanceof IMemoryBlockExtension))
		{		
			// If not extended memory block, do not create any buffer
			// no scrolling
			fContentInput.setPreBuffer(0);
			fContentInput.setPostBuffer(0);
			fContentInput.setDefaultBufferSize(0);
		}
		
		// set up table cursor
		createCursor(fTableViewer.getTable(), fSelectedAddress);
		fTableViewer.getTable().addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				handleTableMouseEvent(e);
			}});
		
		// create pop up menu for the rendering
		createActions();
		createPopupMenu(fTableViewer.getControl());
		createPopupMenu(fTableCursor);
		
		fMenuListener = new IMenuListener() {
					public void menuAboutToShow(IMenuManager manager) {
						fillContextMenu(manager);
						manager.add(new Separator(IWorkbenchActionConstants.MB_ADDITIONS));
					}};
		getPopupMenuManager().addMenuListener(fMenuListener);
		
		// now the rendering is successfully created
		fIsCreated = true;

		//synchronize
		addRenderingToSyncService();
		synchronize();
		
		fTopRowAddress = getTopVisibleAddress();
		// Need to resize column after content is filled in
		// Pack function does not work unless content is not filled in
		// since the table is not able to compute the preferred size.
		resizeColumnsToPreferredSize();
		try {
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
			{
				if(((IMemoryBlockExtension)getMemoryBlock()).getBigBaseAddress() == null)
				{
					DebugException e = new DebugException(DebugUIPlugin.newErrorStatus(DebugUIMessages.AbstractTableRendering_1, null)); 
					displayError(e);				
				}
			}
		} catch (DebugException e1) {
			displayError(e1);	
		}

		// add font change listener and update font when the font has been changed
		JFaceResources.getFontRegistry().addListener(this);
		fScrollbarSelectionListener = new SelectionAdapter() {

			public void widgetSelected(SelectionEvent event) {
				handleScrollBarSelection();
				
			}};
		scroll.addSelectionListener(fScrollbarSelectionListener);
		DebugUIPlugin.getDefault().getPreferenceStore().addPropertyChangeListener(this);
	}
	
	private boolean validateInitialFormat()
	{
		int rowSize = getDefaultRowSize();
		int columnSize = getDefaultColumnSize();
		
		if (rowSize < columnSize || rowSize % columnSize != 0 || rowSize == 0 || columnSize == 0)
		{
			return false;
		}
		return true;
	}

	private BigInteger getInitialTopVisibleAddress() {
		BigInteger topVisibleAddress = (BigInteger) getSynchronizedProperty(AbstractTableRendering.PROPERTY_TOP_ADDRESS);
		if (topVisibleAddress == null)
		{
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
			{
				try {
					topVisibleAddress = ((IMemoryBlockExtension)getMemoryBlock()).getBigBaseAddress();
				} catch (DebugException e1) {
					topVisibleAddress = new BigInteger("0"); //$NON-NLS-1$
				}
			}
			else
			{
				topVisibleAddress = BigInteger.valueOf(getMemoryBlock().getStartAddress());
			}
		}
		return topVisibleAddress;
	}

	private void setupSelectedAddress() {
		// figure out selected address 
		BigInteger selectedAddress = (BigInteger) getSynchronizedProperty(AbstractTableRendering.PROPERTY_SELECTED_ADDRESS);
		if (selectedAddress == null)
		{
			if (getMemoryBlock() instanceof IMemoryBlockExtension) {
				try {
					selectedAddress = ((IMemoryBlockExtension) getMemoryBlock())
							.getBigBaseAddress();
				} catch (DebugException e1) {
					selectedAddress = new BigInteger("0"); //$NON-NLS-1$
				}
				if (selectedAddress == null) {
					selectedAddress = new BigInteger("0"); //$NON-NLS-1$
				}

			} else {
				long address = getMemoryBlock().getStartAddress();
				selectedAddress = BigInteger.valueOf(address);
			}
		}
		setSelectedAddress(selectedAddress);
	}

	private void setupInitialFormat() {
		
		boolean validated = validateInitialFormat();
		
		if (!validated)
		{
			// pop up dialog to ask user for default values
			StringBuffer msgBuffer = new StringBuffer(DebugUIMessages.AbstractTableRendering_20);
			msgBuffer.append(" "); //$NON-NLS-1$
			msgBuffer.append(this.getLabel());
			msgBuffer.append("\n\n"); //$NON-NLS-1$
			msgBuffer.append(DebugUIMessages.AbstractTableRendering_16);
			msgBuffer.append("\n"); //$NON-NLS-1$
			msgBuffer.append(DebugUIMessages.AbstractTableRendering_18);
			msgBuffer.append("\n\n"); //$NON-NLS-1$
			
			int bytePerLine = fBytePerLine;
			int columnSize = fColumnSize;
			
			// initialize this value to populate the dialog properly
			fBytePerLine = getDefaultRowSize() / getAddressableSize();
			fColumnSize = getDefaultColumnSize() / getAddressableSize();

			FormatTableRenderingDialog dialog = new FormatTableRenderingDialog(this, DebugUIPlugin.getShell());
			dialog.openError(msgBuffer.toString());
			
			// restore to original value before formatting
			fBytePerLine = bytePerLine;
			fColumnSize = columnSize;
			
			bytePerLine = dialog.getRowSize() * getAddressableSize();
			columnSize = dialog.getColumnSize() * getAddressableSize();
			
			format(bytePerLine, columnSize);
		}
		else
		{
			// Row size is stored as number of addressable units in preference store
			int bytePerLine = getDefaultRowSize();
			// column size is now stored as number of addressable units
			int columnSize = getDefaultColumnSize();
			
			// format memory block with specified "bytesPerLine" and "columnSize"	
			boolean ok = format(bytePerLine, columnSize);
			
			if (!ok)
			{
				// this is to ensure that the rest of the rendering can be created
				// and we can recover from a format error
				format(bytePerLine, bytePerLine);
			}
		}
	}

	private int getDefaultColumnSize() {
		
		// default to global preference store
		IPreferenceStore prefStore = DebugUITools.getPreferenceStore();
		int columnSize = prefStore.getInt(IDebugPreferenceConstants.PREF_COLUMN_SIZE);
		// actual column size is number of addressable units * size of the addressable unit
		columnSize = columnSize * getAddressableSize();
		
		// check synchronized column size
		Integer colSize = (Integer)getSynchronizedProperty(AbstractTableRendering.PROPERTY_COL_SIZE);
		if (colSize != null)
		{
			// column size is stored as actual number of bytes in synchronizer
			int syncColSize = colSize.intValue(); 
			if (syncColSize > 0)
			{
				columnSize = syncColSize;
			}	
		}
		else
		{
			IPersistableDebugElement elmt = (IPersistableDebugElement)getMemoryBlock().getAdapter(IPersistableDebugElement.class);
			int defaultColSize = -1;
			
			if (elmt != null)
			{
				if (elmt.supportsProperty(this, IDebugPreferenceConstants.PREF_COL_SIZE_BY_MODEL))
					defaultColSize = getDefaultFromPersistableElement(IDebugPreferenceConstants.PREF_COL_SIZE_BY_MODEL);
			}
			
			if (defaultColSize <= 0)
			{
				// if not provided, get default by model
				defaultColSize = getDefaultColumnSizeByModel(getMemoryBlock().getModelIdentifier());
			}
			
			if (defaultColSize > 0)
				columnSize = defaultColSize * getAddressableSize();
		}
		return columnSize;
	}

	private int getDefaultRowSize() {
		
		int rowSize = DebugUITools.getPreferenceStore().getInt(IDebugPreferenceConstants.PREF_ROW_SIZE);
		int bytePerLine = rowSize * getAddressableSize();
		
		// check synchronized row size
		Integer size = (Integer)getSynchronizedProperty(AbstractTableRendering.PROPERTY_ROW_SIZE);
		if (size != null)
		{
			// row size is stored as actual number of bytes in synchronizer
			int syncRowSize = size.intValue(); 
			if (syncRowSize > 0)
			{
				bytePerLine = syncRowSize;
			}	
		}
		else
		{
			int defaultRowSize = -1;
			IPersistableDebugElement elmt = (IPersistableDebugElement)getMemoryBlock().getAdapter(IPersistableDebugElement.class);
			if (elmt != null)
			{
				if (elmt.supportsProperty(this, IDebugPreferenceConstants.PREF_ROW_SIZE_BY_MODEL))
				{
					defaultRowSize = getDefaultFromPersistableElement(IDebugPreferenceConstants.PREF_ROW_SIZE_BY_MODEL);
					return defaultRowSize * getAddressableSize();
				}
			}
			
			if (defaultRowSize <= 0)
				// no synchronized property, ask preference store by id
				defaultRowSize = getDefaultRowSizeByModel(getMemoryBlock().getModelIdentifier());
			
			if (defaultRowSize > 0)
				bytePerLine = defaultRowSize * getAddressableSize();
		}
		return bytePerLine;
	}

	private int getDefaultFromPersistableElement(String propertyId) {
		int defaultValue = -1;
		IPersistableDebugElement elmt = (IPersistableDebugElement)getMemoryBlock().getAdapter(IPersistableDebugElement.class);
		if (elmt != null)
		{
			try {
				Object valueMB = elmt.getProperty(this, propertyId);
				if (valueMB != null && !(valueMB instanceof Integer))
				{
					IStatus status = DebugUIPlugin.newErrorStatus("Model returned invalid type on " + propertyId, null); //$NON-NLS-1$
					DebugUIPlugin.log(status);
				}
				
				if (valueMB != null)
				{
					Integer value = (Integer)valueMB;
					defaultValue = value.intValue();
				}
			} catch (CoreException e) {
				DebugUIPlugin.log(e);
			}
		}
		return defaultValue;
	}
	
	private void getPageSizeFromPreference()
	{
		fPageSize = DebugUIPlugin.getDefault().getPreferenceStore().getInt(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE);
	}
	
	private void createCursor(Table table, BigInteger address)
	{
		fTableCursor = new TableCursor(table, SWT.NONE);
		Display display = fTableCursor.getDisplay();
		
		// set up cursor color
		fTableCursor.setBackground(display.getSystemColor(SWT.COLOR_LIST_SELECTION));
		fTableCursor.setForeground(display.getSystemColor(SWT.COLOR_LIST_SELECTION_TEXT));
		
		fTableCursor.setFont(JFaceResources.getFont(IInternalDebugUIConstants.FONT_NAME));
		fTableCursor.setVisible(true);
		
		fCursorKeyAdapter = new KeyAdapter() {
			public void keyPressed(KeyEvent e)
			 {
			 	handleCursorKeyPressed(e);
			 }	
		};
		fTableCursor.addKeyListener(fCursorKeyAdapter);
		
		fCursorTraverseListener = new TraverseListener() {
			public void keyTraversed(TraverseEvent e) {
				handleCursorTraverseEvt(e);
			}};
					
		fTableCursor.addTraverseListener(fCursorTraverseListener);
		
		fCursorMouseListener = new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				handleCursorMouseEvent(e);
			}};
		fTableCursor.addMouseListener(fCursorMouseListener);
		
		// cursor may be disposed before disposed is called
		// remove listeners whenever the cursor is disposed
		fTableCursor.addDisposeListener(new DisposeListener() {
			public void widgetDisposed(DisposeEvent e) {
				if (fTableCursor == null)
					return;
				fTableCursor.removeTraverseListener(fCursorTraverseListener);
				fTableCursor.removeKeyListener(fCursorKeyAdapter);
				fTableCursor.removeMouseListener(fCursorMouseListener);
				fTableCursor.removeSelectionListener(fCursorSelectionListener);
			}});
		
		fCursorSelectionListener = new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						
						if (!fEvtHandleLock.acquireLock(this))
							return;
	
						handleCursorMoved();
						
						fEvtHandleLock.releaseLock(this);

					}
				};
		fTableCursor.addSelectionListener(fCursorSelectionListener);
		
		
		setCursorAtAddress(address);
		
		fCursorEditor = new TableEditor (fTableViewer.getTable());	
	}
	
	private void handleCursorTraverseEvt(TraverseEvent e){
		
		if (fTableCursor.getRow() == null)
			return;
		
		Table table = (Table)fTableCursor.getParent();
		int row = table.indexOf(fTableCursor.getRow());
		int col = fTableCursor.getColumn();
		if (col == getNumCol() && e.keyCode == SWT.ARROW_RIGHT)
		{
			if (row + 1>= table.getItemCount())
			{
				return;
			}
			
			row = row +1;
			col = 0;
			fTableCursor.setSelection(row, col);
		}
		if (col <= 1 && e.keyCode == SWT.ARROW_LEFT)
		{
			if (row-1 < 0)
			{
				return;
			}
			
			row = row - 1;
			col = getNumCol()+1;
			fTableCursor.setSelection(row, col);
		}			
		
		Object evtLockClient = new Object();
		if (!fEvtHandleLock.acquireLock(evtLockClient))
			return;
		
		handleCursorMoved();
		
		fEvtHandleLock.releaseLock(evtLockClient);

	}
	
	/**
	 * Update selected address.
	 * Load more memory if required.
	 */
	private void handleCursorMoved()
	{	
		if (fIsDisposed)
			return;
		
		BigInteger selectedAddress = getSelectedAddressFromCursor(fTableCursor);
		
		// when the cursor is moved, the selected address is changed
		if (selectedAddress != null && !selectedAddress.equals(fSelectedAddress))
		{
			setSelectedAddress(selectedAddress);
			updateSyncSelectedAddress();
		}
		
		// now check to see if the cursor is approaching buffer limit
		TableItem item = fTableCursor.getRow();
		if (item == null)
			return;
		
		if (getMemoryBlock() instanceof IMemoryBlockExtension)
		{
			int row = fTableViewer.getTable().indexOf(item);
			
			if (row < 3)
			{
				if (!isAtTopLimit())
				{
					if (isDynamicLoad())
					{
						refresh();
						setCursorAtAddress(fSelectedAddress);
					}
				}
			}
			else if (row >= fTableViewer.getTable().getItemCount() - 3)
			{
				if (!isAtBottomLimit())
				{
					if (isDynamicLoad())
					{
						refresh();
						setCursorAtAddress(fSelectedAddress);
					}
				}
			}
		}
		
		// if the cursor has moved, the top index of the table may change
		// just update the synchronization service
		BigInteger address = getTopVisibleAddress();
		if (!address.equals(fTopRowAddress))
		{
			fTopRowAddress = address;
			updateSyncTopAddress();
		}
	}
	
	private void handleCursorKeyPressed(KeyEvent event)
	{
		// allow edit if user hits return
		if (event.character == '\r' && event.getSource() instanceof TableCursor)
		{
			activateCellEditor(null);
			return;
		}		
		
		if (MemoryViewUtil.isValidEditEvent(event.keyCode))
		{	
			// activate edit as soon as user types something at the cursor
			if (event.getSource() instanceof TableCursor)
			{
				String initialValue = String.valueOf(event.character);
				activateCellEditor(initialValue);
				return;
			}
		}
	}
	
	/**
	 * Calculate selected address based on cursor's current position
	 * @param cursor
	 * @return the selected address
	 */
	private BigInteger getSelectedAddressFromCursor(TableCursor cursor)
	{
		TableItem row = cursor.getRow();
		int col = cursor.getColumn();
		
		return getAddressFromTableItem(row, col);
	}

	private BigInteger getAddressFromTableItem(TableItem row, int col) {
		if (row == null)
			return null;
		
		// get row address
		String temp = ((TableRenderingLine)row.getData()).getAddress();
		BigInteger rowAddress = new BigInteger(temp, 16);
		
		int offset;
		if (col > 0)
		{	
			// 	get address offset
			int addressableUnit = getAddressableUnitPerColumn();
			offset = (col-1) * addressableUnit;
		}
		else
		{
			offset = 0;
		}
		
		return rowAddress.add(BigInteger.valueOf(offset));
	}
	
	
	/**
	 * Sets the cursor at the specified address
	 * @param address
	 * @return true if successful, false otherwise
	 */
	private boolean setCursorAtAddress(BigInteger address)
	{
		// selected address is out of range, simply return false
		if (address.compareTo(fContentProvider.getBufferTopAddress()) < 0)
			return false;
		
		// calculate selected row address
		int addressableUnit = getAddressableUnitPerLine();
		int numOfRows = address.subtract(fContentProvider.getBufferTopAddress()).intValue()/addressableUnit;
		BigInteger rowAddress = fContentProvider.getBufferTopAddress().add(BigInteger.valueOf(numOfRows * addressableUnit));

		// try to find the row of the selected address
		int row = findAddressIndex(address);
			
		if (row == -1)
		{
			return false;
		}
		
		// calculate offset to the row address
		BigInteger offset = address.subtract(rowAddress);
		
		// locate column
		int colAddressableUnit = getAddressableUnitPerColumn();
		int col = ((offset.intValue()/colAddressableUnit)+1);
		
		if (col == 0)
			col = 1;
		
		fTableCursor.setSelection(row, col);
		
		return true;		
	}
	
	
	/**
	 * Format view tab based on the bytes per line and column.
	 * 
	 * @param bytesPerLine - number of bytes per line, possible values: (1 / 2 / 4 / 8 / 16) * addressableSize
	 * @param columnSize - number of bytes per column, possible values: (1 / 2 / 4 / 8 / 16) * addressableSize
	 * @return true if format is successful, false, otherwise
	 */
	public boolean format(int bytesPerLine, int columnSize)
	{	
		
		// selected address gets changed as the cursor is moved
		// during the reformat.
		// Back up the address and restore it later.
		BigInteger selectedAddress = fSelectedAddress;
		
		// bytes per cell must be divisible to bytesPerLine
		if (bytesPerLine % columnSize != 0)
		{
			return false;
		}
		
		if (bytesPerLine < columnSize)
		{
			return false;
		}
		
		// do not format if the view tab is already in that format
		if(fBytePerLine == bytesPerLine && fColumnSize == columnSize){
			return false;
		}
		
		fBytePerLine = bytesPerLine;
		fColumnSize = columnSize;
		
		Object evtLockClient = new Object();
		if (!fEvtHandleLock.acquireLock(evtLockClient))
			return false;
		
		// if the tab is already created and is being reformatted
		if (fIsCreated)
		{	
			if (fTableViewer == null)
				return false;
			
			if (fTableViewer.getTable() == null)
				return false;
			
			// clean up old columns
			TableColumn[] oldColumns = fTableViewer.getTable().getColumns();
			
			for (int i=0; i<oldColumns.length; i++)
			{
				oldColumns[i].dispose();
			}
			
			// clean up old cell editors
			CellEditor[] oldCellEditors = fTableViewer.getCellEditors();
			
			for (int i=0; i<oldCellEditors.length; i++)
			{
				oldCellEditors[i].dispose();
			}
		}
		
		TableColumn column0 = new TableColumn(fTableViewer.getTable(),SWT.LEFT,0);
		column0.setText(DebugUIMessages.AbstractTableRendering_2); 
		
		// create new byte columns
		TableColumn [] byteColumns = new TableColumn[bytesPerLine/columnSize];		
		
		String[] columnLabels = new String[0];
		IMemoryBlockTablePresentation presentation = getTablePresentationAdapter();
		if (presentation != null)
		{
			columnLabels = presentation.getColumnLabels(getMemoryBlock(), bytesPerLine, getNumCol());
		}
		
		// check that column labels are not null
		if (columnLabels == null)
			columnLabels = new String[0];
		
		for (int i=0;i<byteColumns.length; i++)
		{
			TableColumn column = new TableColumn(fTableViewer.getTable(), SWT.LEFT, i+1);
			
			// if the number of column labels returned is correct
			// use supplied column labels
			if (columnLabels.length == byteColumns.length)
			{
				column.setText(columnLabels[i]);
			}
			else
			{
				// otherwise, use default
				int addressableUnit = columnSize/getAddressableSize();
				if (getAddressableUnitPerColumn() >= 4)
				{
					column.setText(Integer.toHexString(i*addressableUnit).toUpperCase() + 
						" - " + Integer.toHexString(i*addressableUnit+addressableUnit-1).toUpperCase()); //$NON-NLS-1$
				}
				else
				{
					column.setText(Integer.toHexString(i*addressableUnit).toUpperCase());
				}
			}
		}
		
		//Empty column for cursor navigation
		TableColumn emptyCol = new TableColumn(fTableViewer.getTable(),SWT.LEFT,byteColumns.length+1);
		emptyCol.setText(" "); //$NON-NLS-1$
		emptyCol.setWidth(1);
		emptyCol.setResizable(false);

		// +2 to include properties for address and navigation column
		String[] columnProperties = new String[byteColumns.length+2];
		columnProperties[0] = TableRenderingLine.P_ADDRESS;
		
		int addressableUnit = columnSize / getAddressableSize();

		// use column beginning offset to the row address as properties
		for (int i=1; i<columnProperties.length-1; i++)
		{
			// column properties are stored as number of addressable units from the
			// the line address
			columnProperties[i] = Integer.toHexString((i-1)*addressableUnit);
		}
		
		// Empty column for cursor navigation
		columnProperties[columnProperties.length-1] = " "; //$NON-NLS-1$
		
		fTableViewer.setColumnProperties(columnProperties);		
		
		
		Table table = fTableViewer.getTable();
		fEditors = new CellEditor[table.getColumnCount()];
		for (int i=0; i<fEditors.length; i++)
		{
			fEditors[i] = new TextCellEditor(table);
		}
		
		// create and set cell editors
		fTableViewer.setCellEditors(fEditors);	
		
		if (fIsCreated)
		{
			fTableViewer.refresh();
		}		
		
		resizeColumnsToPreferredSize();
		updateSyncRowSize();
		updateSyncColSize();
		
		if (fIsCreated)
		{
			// for Linux GTK, this must happen after table viewer is refreshed
			int i = findAddressIndex(fTopRowAddress);
			
			if (i >= 0)
				setTopIndex(fTableViewer.getTable(), i);
			
			if (isAddressVisible(selectedAddress))
				// after refresh, make sure the cursor is at the correct position
				setCursorAtAddress(selectedAddress);			
		}
		
		fEvtHandleLock.releaseLock(evtLockClient);
		
		return true;
	}
	
	/**
	 * Create the error page for this rendering.
	 * The error page is used to report any error resulted from
	 * getting memory from a memory block.
	 * @param parent
	 */
	private void createErrorPage(Composite parent)
	{
		if (fTextViewer == null)
		{
			fTextViewer = new TextViewer(parent, SWT.WRAP);	
			fTextViewer.setDocument(new Document());
			StyledText styleText = fTextViewer.getTextWidget();
			styleText.setEditable(false);
			styleText.setEnabled(false);
		}
	}
	
	/**
	 * Displays the content of the table viewer.
	 */
	public void displayTable()
	{
		fIsShowingErrorPage = false;
		fPageBook.showPage(fTableViewer.getControl());
	}
	
	/**
	 * Displays an error message for the given exception.
	 * 
	 * @param e exception to display 
	 */
	public void displayError(DebugException e)
	{
		StyledText styleText = null;
		fIsShowingErrorPage = true;

		styleText = fTextViewer.getTextWidget();
		
		if (styleText != null)
			styleText.setText(DebugUIMessages.AbstractTableRendering_3 + e.getMessage());
		fPageBook.showPage(fTextViewer.getControl());
		
		// clear content cache if we need to display error
		fContentProvider.clearContentCache();
	}
	
	/**
	 * Returns whether the error page is displayed.
	 * 
	 * @return whether the error page is displayed
	 */
	public boolean isDisplayingError()
	{	
		return fIsShowingErrorPage;
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#getControl()
	 */
	public Control getControl() {
		return fPageBook;
	}
	
	/**
	 * Returns the addressable size of this rendering's memory block in bytes.
	 * 
	 * @return the addressable size of this rendering's memory block in bytes
	 */
	public int getAddressableSize() {
		return fAddressableSize;
	}
	
	private Object getSynchronizedProperty(String propertyId)
	{
		IMemoryRenderingSynchronizationService syncService = getMemoryRenderingContainer().getMemoryRenderingSite().getSynchronizationService();
		
		if (syncService == null)
			return null;
		
		return syncService.getProperty(getMemoryBlock(), propertyId);	
	}
	
	/**
	 * This method estimates the number of visible lines in the rendering
	 * table.  
	 * @return estimated number of visible lines in the table
	 */
	private int getNumberOfVisibleLines()
	{
		if(fTableViewer == null)
			return -1;
		
		Table table = fTableViewer.getTable();
		int height = fTableViewer.getTable().getSize().y;
		
		// when table is not yet created, height is zero
		if (height == 0)
		{
			// make use of the table viewer to estimate table size
			height = fTableViewer.getTable().getParent().getSize().y;
		}
		
		// height of border
		int border = fTableViewer.getTable().getHeaderHeight();
		
		// height of scroll bar
		int scroll = fTableViewer.getTable().getHorizontalBar().getSize().y;

		// height of table is table's area minus border and scroll bar height		
		height = height-border-scroll;

		// calculate number of visible lines
		int lineHeight = getMinTableItemHeight(table);
		
		int numberOfLines = height/lineHeight;
		
		if (numberOfLines <= 0)
			return 20;
	
		return numberOfLines;		
	}
	
	private static void  setTopIndex(Table table, int index)
	{
		table.setTopIndex(index);
	}

	private void addRenderingToSyncService()
	{	
		IMemoryRenderingSynchronizationService syncService = getMemoryRenderingContainer().getMemoryRenderingSite().getSynchronizationService();
		
		if (syncService == null)
			return;
		
		syncService.addPropertyChangeListener(this, null);
	
		// we could be in a format error even though the error is not yet displayed
		// do not update sync property in this case
		if (!isDisplayingError())
		{
			if (syncService.getSynchronizationProvider() == null)
				syncService.setSynchronizationProvider(this);
			
			// check if there is already synchronization info available
			Object selectedAddress =getSynchronizedProperty( AbstractTableRendering.PROPERTY_SELECTED_ADDRESS);
			Object rowSize = getSynchronizedProperty(AbstractTableRendering.PROPERTY_ROW_SIZE);
			Object colSize =getSynchronizedProperty( AbstractTableRendering.PROPERTY_COL_SIZE);
			Object topAddress =getSynchronizedProperty( AbstractTableRendering.PROPERTY_TOP_ADDRESS);
			
			if (!isDynamicLoad())
			{
				Object pageStartAddress = getSynchronizedProperty(IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS);
				if (pageStartAddress == null)
					updateSyncPageStartAddress();
			}
			
			// if info is available, some other view tab has already been
			// created
			// do not overwrite info in the synchronizer if that's the case
			if (selectedAddress == null) {
				updateSyncSelectedAddress();
			}
			
			if (rowSize == null)
			{
				updateSyncRowSize();
			}

			if (colSize == null) {
				updateSyncColSize();
			}
			if (topAddress == null) {
				updateSyncTopAddress();
			}
		}
	}
	
	/**
	 * Get properties from synchronizer and synchronize settings
	 */
	private void synchronize()
	{			
		if (!isDynamicLoad())
		{
			BigInteger pageStart = (BigInteger)getSynchronizedProperty(IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS);
			if (pageStart != null && fContentInput != null && fContentInput.getLoadAddress() != null)
			{
				if (!fContentInput.getLoadAddress().equals(pageStart))
					handlePageStartAddressChanged(pageStart);
			}
			else if (pageStart != null)
			{
				handlePageStartAddressChanged(pageStart);
			}
		}
		
		Integer rowSize = (Integer) getSynchronizedProperty(AbstractTableRendering.PROPERTY_ROW_SIZE);
		Integer columnSize = (Integer) getSynchronizedProperty(AbstractTableRendering.PROPERTY_COL_SIZE);
		BigInteger selectedAddress = (BigInteger)getSynchronizedProperty(AbstractTableRendering.PROPERTY_SELECTED_ADDRESS);
		BigInteger topAddress = (BigInteger)getSynchronizedProperty(AbstractTableRendering.PROPERTY_TOP_ADDRESS);
		
		if (rowSize != null)
		{
			int rSize = rowSize.intValue();
			if (rSize > 0 && rSize != fBytePerLine) {
				rowSizeChanged(rSize);
			}
		}
		
		if (columnSize != null) {
			int colSize = columnSize.intValue();
			if (colSize > 0 && colSize != fColumnSize) {
				columnSizeChanged(colSize);
			}
		}
		if (topAddress != null) {
			if (!topAddress.equals(getTopVisibleAddress())) {
				if (selectedAddress != null) {
					if (!fSelectedAddress.equals(selectedAddress)) {
						selectedAddressChanged(selectedAddress);
					}
				}
				topVisibleAddressChanged(topAddress, false);
			}
		}
		if (selectedAddress != null) {
			if (selectedAddress.compareTo(fSelectedAddress) != 0) {
				selectedAddressChanged(selectedAddress);
			}
		}
	}
	
	/**
	 * Resize column to the preferred size.
	 */
	public void resizeColumnsToPreferredSize() {
		// pack columns
		Table table = fTableViewer.getTable();
		TableColumn[] columns = table.getColumns();
		
		for (int i=0 ;i<columns.length-1; i++)
		{	
			columns[i].pack();
		}
		
		if (!fIsShowAddressColumn)
		{
			columns[0].setWidth(0);
		}
	}
	
	/**
	 * update selected address in synchronizer if update is true.
	 */
	private void updateSyncSelectedAddress() {
		
		if (!fIsCreated)
			return;
		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractTableRendering.PROPERTY_SELECTED_ADDRESS, null, fSelectedAddress);
		firePropertyChangedEvent(event);
	}

	/**
	 * update column size in synchronizer
	 */
	private void updateSyncColSize() {
		
		if (!fIsCreated)
			return;
		
		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractTableRendering.PROPERTY_COL_SIZE, null, new Integer(fColumnSize));
		firePropertyChangedEvent(event);
	}
	
	/**
	 * update column size in synchronizer
	 */
	private void updateSyncRowSize() {
		
		if (!fIsCreated)
			return;
		
		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractTableRendering.PROPERTY_ROW_SIZE, null, new Integer(fBytePerLine));
		firePropertyChangedEvent(event);
	}
	
	/**
	 * update top visible address in synchronizer
	 */
	private void updateSyncTopAddress() {
		
		if (!fIsCreated)
			return;

		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractTableRendering.PROPERTY_TOP_ADDRESS, null, fTopRowAddress);
		firePropertyChangedEvent(event);
	}
	
	private void updateSyncPageStartAddress() {
	
		if (!fIsCreated)
			return;
		
		if (isBaseAddressChanged())
			return;
		
		BigInteger pageStart;
		if (isDynamicLoad())
		{
			// if dynamic loading, the page address should be the top
			// row address
			pageStart = fTopRowAddress;
		}
		else
		{
			// otherwise, the address is the buffer's start address
			pageStart = fContentProvider.getBufferTopAddress();
		}
		
		PropertyChangeEvent event = new PropertyChangeEvent(this, IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS, null, pageStart);
		firePropertyChangedEvent(event);
	}
	
	/**
	 * Fills the context menu for this rendering
	 * 
	 * @param menu menu to fill
	 */
	protected void fillContextMenu(IMenuManager menu) {
	
		menu.add(new Separator("topMenu")); //$NON-NLS-1$
		menu.add(fResetMemoryBlockAction);
		menu.add(fGoToAddressAction);
	
		menu.add(new Separator());
		
		menu.add(fFormatRenderingAction);

		if (!isDynamicLoad() && getMemoryBlock() instanceof IMemoryBlockExtension)
		{		
			menu.add(new Separator());
			menu.add(fPrevAction);
			menu.add(fNextAction);
		}
		
		menu.add(new Separator());
		menu.add(fReformatAction);
		menu.add(fToggleAddressColumnAction);
		menu.add(new Separator());
		menu.add(fCopyToClipboardAction);
		menu.add(fPrintViewTabAction);
		if (fPropertiesAction != null)
		{
			menu.add(new Separator());
			menu.add(fPropertiesAction);
		}
		
	}
	
	/**
	 * Returns the number of addressable units per row.
	 *  
	 * @return number of addressable units per row
	 */
	public int getAddressableUnitPerLine() {
		return fBytePerLine / getAddressableSize();
	}
	
	/**
	 * Returns the number of addressable units per column.
	 * 
	 * @return number of addressable units per column
	 */
	public int getAddressableUnitPerColumn() {
		return fColumnSize / getAddressableSize();
	}
	
	/**
	 * Returns the number of bytes displayed in a single column cell.
	 * 
	 * @return the number of bytes displayed in a single column cell
	 */
	public int getBytesPerColumn()
	{
		return fColumnSize;
	}

	/**
	 * Returns the number of bytes displayed in a row.
	 * 
	 * @return the number of bytes displayed in a row
	 */
	public int getBytesPerLine()
	{		
		return fBytePerLine;
	}
	
	/**
	 * Updates labels of this rendering.
	 */
	public void updateLabels()
	{
		// update tab labels
		updateRenderingLabel(true);
		
		if (fTableViewer != null)
		{
			// update column labels
			setColumnHeadings();
			fTableViewer.refresh();
		}
	}
	

	/* Returns the label of this rendering.
	 * 
	 * @return label of this rendering
	 */
	public String getLabel() {
		if (fLabel == null)
			fLabel = buildLabel(true);
		
		return fLabel;
	}

	
	/**
	 * Updates the label of this rendering, optionally displaying the
	 * base address of this rendering's memory block.
	 * 
	 * @param showAddress whether to display the base address of this
	 *  rendering's memory block in this rendering's label
	 */
	protected void updateRenderingLabel(boolean showAddress)
	{	
		fLabel = buildLabel(showAddress);
		firePropertyChangedEvent(new PropertyChangeEvent(this, IBasicPropertyConstants.P_TEXT, null, fLabel));
	}

	private String buildLabel(boolean showAddress) {
		String label = ""; //$NON-NLS-1$
		if (getMemoryBlock() instanceof IMemoryBlockExtension)
		{
			label = ((IMemoryBlockExtension)getMemoryBlock()).getExpression();
			
			if (label.startsWith("&")) //$NON-NLS-1$
				label = "&" + label; //$NON-NLS-1$
			
			if (label == null)
			{
				label = DebugUIMessages.AbstractTableRendering_8; 
			}
			
			try {
				if (showAddress && ((IMemoryBlockExtension)getMemoryBlock()).getBigBaseAddress() != null)
				{	
					label += " : 0x"; //$NON-NLS-1$
					label += ((IMemoryBlockExtension)getMemoryBlock()).getBigBaseAddress().toString(16).toUpperCase();
				}
			} catch (DebugException e) {
				// do nothing, the label will not show the address
			}
		}
		else
		{
			long address = getMemoryBlock().getStartAddress();
			label = Long.toHexString(address).toUpperCase();
		}
		
		String preName = DebugUITools.getMemoryRenderingManager().getRenderingType(getRenderingId()).getLabel();
		
		if (preName != null)
			label += " <" + preName + ">"; //$NON-NLS-1$ //$NON-NLS-2$
		
		return decorateLabel(label);
	}
	
	private void setColumnHeadings()
	{
		String[] columnLabels = new String[0];
		
		IMemoryBlockTablePresentation presentation = getTablePresentationAdapter();
		if (presentation != null)
		{
			columnLabels = presentation.getColumnLabels(getMemoryBlock(), fBytePerLine, getNumCol());
		}
		
		// check that column labels returned are not null
		if (columnLabels == null)
			columnLabels = new String[0];
		
		int numByteColumns = fBytePerLine/fColumnSize;
		
		TableColumn[] columns = fTableViewer.getTable().getColumns();
		
		int j=0;
		for (int i=1; i<columns.length-1; i++)
		{	
			// if the number of column labels returned is correct
			// use supplied column labels
			if (columnLabels.length == numByteColumns)
			{
				columns[i].setText(columnLabels[j]);
				j++;
			}
			else
			{
				// otherwise, use default
				if (fColumnSize >= 4)
				{
					columns[i].setText(Integer.toHexString(j*fColumnSize).toUpperCase() + 
							" - " + Integer.toHexString(j*fColumnSize+fColumnSize-1).toUpperCase()); //$NON-NLS-1$
				}
				else
				{
					columns[i].setText(Integer.toHexString(j*fColumnSize).toUpperCase());
				}
				j++;
			}
		}
	}
	
	/**
	 * Refresh the table viewer with the current top visible address.
	 * Update labels in the memory rendering.
	 */
	public void refresh()
	{	
		// refresh at start address of this memory block
		// address may change if expression is evaluated to a different value
		IMemoryBlock mem = getMemoryBlock();
		BigInteger address;
		
		if (mem instanceof IMemoryBlockExtension)
		{
			try {
				address = ((IMemoryBlockExtension)mem).getBigBaseAddress();
				if (address == null)
				{	
					DebugException e = new DebugException(DebugUIPlugin.newErrorStatus(DebugUIMessages.AbstractTableRendering_10, null)); 
					displayError(e);
					return;
				}
				updateRenderingLabel(true);
				// base address has changed
				if (address.compareTo(fContentProvider.getContentBaseAddress()) != 0)
				{
					// get to new address
					setSelectedAddress(address);
					updateSyncSelectedAddress();
					
					reloadTable(address, true);
					
					if (!isDynamicLoad())
					{
						updateSyncPageStartAddress();
						setTopIndex(fTableViewer.getTable(), 0);
					}
					
					fTopRowAddress = getTopVisibleAddress();
					updateSyncTopAddress();
					
					fContentInput.updateContentBaseAddress();
				}
				else
				{
					// reload at top of table
					if (isDynamicLoad())
						address = getTopVisibleAddress();
					else
						address = fContentInput.getLoadAddress();
					reloadTable(address, true);
				}
			} catch (DebugException e) {
				displayError(e);
				return;
			}				
		}
		else
		{
			address = BigInteger.valueOf(mem.getStartAddress());
			reloadTable(address, true);
		}
	}
	
	synchronized private void reloadTable(BigInteger topAddress, boolean updateDelta){
		
		if (fTableViewer == null)
			return;
		
		try
		{
			Table table = (Table)fTableViewer.getControl();	
			
			TableRenderingContentInput input;
			if (isDynamicLoad())
				input = new TableRenderingContentInput(this, fContentInput.getPreBuffer(), fContentInput.getPostBuffer(), fContentInput.getDefaultBufferSize(), topAddress, getNumberOfVisibleLines(), updateDelta, null);
			else
				input = new TableRenderingContentInput(this, fContentInput.getPreBuffer(), fContentInput.getPostBuffer(), fContentInput.getDefaultBufferSize(), topAddress, fPageSize, updateDelta, null);
			
			fContentInput = input;
			fTableViewer.setInput(fContentInput);
	
			if (isDynamicLoad())
			{
				if (getMemoryBlock() instanceof IMemoryBlockExtension)
				{
					int topIdx = findAddressIndex(topAddress);
					
					if (topIdx != -1)
					{
						setTopIndex(table, topIdx);
					}
				}
				
				// cursor needs to be refreshed after reload
				if (isAddressVisible(fSelectedAddress))
					setCursorAtAddress(fSelectedAddress);
			}
			else
			{
				if (!isAddressOutOfRange(fSelectedAddress))
				{
					setCursorAtAddress(fSelectedAddress);
					fTableCursor.setVisible(true);
				}
				else
				{
					fTableCursor.setVisible(false);
				}
			}
		}
		finally
		{
		}
	}
	
	private BigInteger getTopVisibleAddress() {
		
		if (fTableViewer == null)
			return BigInteger.valueOf(0);

		Table table = fTableViewer.getTable();
		int topIndex = getTopVisibleIndex(table);

		if (topIndex < 1) { topIndex = 0; }

		if (table.getItemCount() > topIndex) 
		{
			TableRenderingLine topItem = (TableRenderingLine)table.getItem(topIndex).getData();
			
			String calculatedAddress = null;
			if (topItem == null)
			{
				calculatedAddress = table.getItem(topIndex).getText();
			}
			else
			{
				calculatedAddress = topItem.getAddress();				
			}
			
			BigInteger bigInt = new BigInteger(calculatedAddress, 16);
			return bigInt;
		}
		return BigInteger.valueOf(0);
	}
	
	private int findAddressIndex(BigInteger address)
	{
		TableItem items[] = fTableViewer.getTable().getItems();
	
		for (int i=0; i<items.length; i++){
			
			// Again, when the table resizes, the table may have a null item
			// at then end.  This is to handle that.
			if (items[i] != null)
			{	
				TableRenderingLine line = (TableRenderingLine)items[i].getData();
				BigInteger lineAddress = new BigInteger(line.getAddress(), 16);
				int addressableUnit = getAddressableUnitPerLine();
				BigInteger endLineAddress = lineAddress.add(BigInteger.valueOf(addressableUnit));
				
				if (lineAddress.compareTo(address) <= 0 && endLineAddress.compareTo(address) > 0)
				{	
					return i;
				}
			}
		}
		
		return -1;
	}
	
	private static int getTopVisibleIndex(Table table)
	{
		int index = table.getTopIndex();
		
		TableItem item = table.getItem(index);
		int cnt = table.getItemCount();
		
		while (item.getBounds(0).y < 0)
		{
			index++;
			if (index >= cnt)
			{
				index--;
				break;
			}
			item = table.getItem(index);
		}
		
		return index;
	}
	
	/**
	 * Returns this rendering's table viewer.
	 */
	public TableViewer getTableViewer()
	{
		return fTableViewer;
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#dispose()
	 */
	public void dispose() {
		try {	
			// prevent rendering from being disposed again
			if (fIsDisposed)
				return;
			
			fIsDisposed = true;
			
			if (fContentProvider != null)
				fContentProvider.dispose();
			
			ScrollBar scroll = ((Table)fTableViewer.getControl()).getVerticalBar();
			if (scroll != null && !scroll.isDisposed())
				scroll.removeSelectionListener(fScrollbarSelectionListener);
			
			if (!fTableCursor.isDisposed())
			{
				fTableCursor.removeTraverseListener(fCursorTraverseListener);
				fTableCursor.removeKeyListener(fCursorKeyAdapter);
				fTableCursor.removeMouseListener(fCursorMouseListener);
			}
			
			fCursorEditor.dispose();
			
			fTextViewer = null;
			fTableViewer = null;
			fTableCursor = null;
			
			// clean up cell editors
			for (int i=0; i<fEditors.length; i++)
			{
				fEditors[i].dispose();
			}
			
			// remove font change listener when the view tab is disposed
			JFaceResources.getFontRegistry().removeListener(this);
			
			// remove the view tab from the synchronizer
			IMemoryRenderingSynchronizationService syncService = getMemoryRenderingContainer().getMemoryRenderingSite().getSynchronizationService();
			if (syncService != null)
				syncService.removePropertyChangeListener(this);
			
			DebugUIPlugin.getDefault().getPreferenceStore().removePropertyChangeListener(this);
			
			fToolTipShell.dispose();
			
			if (getPopupMenuManager() != null)
			{
				getPopupMenuManager().removeMenuListener(fMenuListener);
			}
			
			super.dispose();

		} catch (Exception e) {}
	}
	
	private int getNumCol() {
		
		int bytesPerLine = getBytesPerLine();
		int columnSize = getBytesPerColumn();
		
		return bytesPerLine/columnSize;
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.IMemoryViewTab#setFont(org.eclipse.swt.graphics.Font)
	 */
	private void setFont(Font font)
	{	
		int oldIdx = getTopVisibleIndex(fTableViewer.getTable());
		
		// BUG in table, if font is changed when table is not starting
		// from the top, causes table gridline to be misaligned.
		setTopIndex(fTableViewer.getTable(),  0);
		
		// set font
		fTableViewer.getTable().setFont(font);
		fTableCursor.setFont(font);
		
		setTopIndex(fTableViewer.getTable(),  oldIdx);
		
		resizeColumnsToPreferredSize();
		
		// update table cursor and force redraw
		setCursorAtAddress(fSelectedAddress);
	}
	
	
	/**
	 * Moves the cursor to the specified address.
	 * Will load more memory if the address is not currently visible.
	 * 
	 * @param address address to position cursor at
	 * @throws DebugException if an exception occurs
	 */
	public void goToAddress(BigInteger address) throws DebugException {
		Object evtLockClient = new Object();
		try
		{	
			if (!fEvtHandleLock.acquireLock(evtLockClient))
				return;

			// if address is within the range, highlight			
			if (!isAddressOutOfRange(address))
			{
				setSelectedAddress(address);
				updateSyncSelectedAddress();
				setCursorAtAddress(fSelectedAddress);
				
				// force the cursor to be shown
				if (!isAddressVisible(fSelectedAddress))
				{	
					int i = findAddressIndex(fSelectedAddress);
					fTableViewer.getTable().showItem(fTableViewer.getTable().getItem(i));
				}
			}
			else
			{
				// if not extended memory block
				// do not allow user to go to an address that's out of range
				if (!(getMemoryBlock() instanceof IMemoryBlockExtension))
				{
					Status stat = new Status(
					 IStatus.ERROR, DebugUIPlugin.getUniqueIdentifier(),
					 DebugException.NOT_SUPPORTED, DebugUIMessages.AbstractTableRendering_11, null  
					);
					DebugException e = new DebugException(stat);
					throw e;
				}

				BigInteger startAdd = fContentInput.getStartAddress();
				BigInteger endAdd = fContentInput.getEndAddress();
				
				if (address.compareTo(startAdd) < 0 ||
					address.compareTo(endAdd) > 0)
				{
					Status stat = new Status(
					 IStatus.ERROR, DebugUIPlugin.getUniqueIdentifier(),
					 DebugException.NOT_SUPPORTED, DebugUIMessages.AbstractTableRendering_11, null  
					);
					DebugException e = new DebugException(stat);
					throw e;
				}
				
				setSelectedAddress(address);
				updateSyncSelectedAddress();
				
				reloadTable(address, false);
				
				if (!isDynamicLoad())
				{						
					updateSyncPageStartAddress();
				}
				
				// if the table is reloaded, the top address is changed in this case
				fTopRowAddress = address;
				updateSyncTopAddress();
				
				// set the cursor at the selected address after reload
				setCursorAtAddress(address);
			}
			fTableCursor.setVisible(true);
		}
		catch (DebugException e)
		{
			throw e;
		}
		finally
		{
			fEvtHandleLock.releaseLock(evtLockClient);
		}
	}
	
	/**
	 * Check if address provided is out of buffered range
	 * @param address
	 * @return if address is out of buffered range
	 */
	private boolean isAddressOutOfRange(BigInteger address)
	{
		return fContentProvider.isAddressOutOfRange(address);
	}
	
	/**
	 * Check if address is visible
	 * @param address
	 * @return if the given address is visible
	 */
	private boolean isAddressVisible(BigInteger address)
	{
		// if view tab is not yet created 
		// cursor should always be visible
		if (!fIsCreated)
			return true;
		
		BigInteger topVisible = getTopVisibleAddress();
		int addressableUnit = getAddressableUnitPerLine();
		BigInteger lastVisible = getTopVisibleAddress().add(BigInteger.valueOf((getNumberOfVisibleLines() * addressableUnit) + addressableUnit));
		
		if (topVisible.compareTo(address) <= 0 && lastVisible.compareTo(address) > 0)
		{
			return true;
		}
		return false;
	}
	
	/**
	 * Create actions for this rendering
	 */
	protected void createActions() {
		fCopyToClipboardAction = new CopyTableRenderingToClipboardAction(this, fTableViewer);
		fGoToAddressAction = new GoToAddressAction(this);
		fResetMemoryBlockAction = new ResetToBaseAddressAction(this);
		fPrintViewTabAction = new PrintTableRenderingAction(this, fTableViewer);
		
		fFormatRenderingAction = new FormatTableRenderingAction(this);		
		fReformatAction = new ReformatAction(this);
		fToggleAddressColumnAction = new ToggleAddressColumnAction();
		
		IMemoryRenderingSite site = getMemoryRenderingContainer().getMemoryRenderingSite();
		if (site.getSite().getSelectionProvider() != null)
		{
			fPropertiesAction = new PropertyDialogAction(site.getSite(),site.getSite().getSelectionProvider()); 
		}
		
		fNextAction = new NextPageAction();
		fPrevAction = new PrevPageAction();
	}
	
	/**
	 * Handle scrolling and reload table if necessary
	 * @param event
	 */
	private synchronized void handleScrollBarSelection()
	{
		Object evtLockClient = new Object();
		try
		{			
			if (fIsDisposed)
				return;
			
			BigInteger address = getTopVisibleAddress();
	
			if (!fTopRowAddress.equals(address))
			{
				fTopRowAddress = address;
				updateSyncTopAddress();
			}
			
			if (!fEvtHandleLock.acquireLock(evtLockClient))
				return;
			
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
			{

				if (isDynamicLoad())
				{
					if (!isAddressOutOfRange(address))
					{
						Table table = fTableViewer.getTable();
						int numInBuffer = table.getItemCount();
						int index = findAddressIndex(address);
						if (index < 3)
						{
							if (isAtTopLimit())
							{
								setTopIndex(table, index);
							}
							else
							{
								reloadTable(address, false);
							}
						}
						else if ((numInBuffer-(index+getNumberOfVisibleLines())) < 3)
						{
							if (!isAtBottomLimit())
								reloadTable(address, false);
						}
					}
					else
					{	
						// approaching limit, reload table
						reloadTable(address, false);
					}
				}
				
				if (isAddressVisible(fSelectedAddress))
					fTableCursor.setVisible(true);
				else
					fTableCursor.setVisible(false);
			}
		}
		finally
		{
			fEvtHandleLock.releaseLock(evtLockClient);
		}
	}
	
	
	private boolean isAtTopLimit()
	{	
		BigInteger startAddress = fContentInput.getStartAddress();
		startAddress = MemoryViewUtil.alignToBoundary(startAddress, getAddressableUnitPerLine() );
		
		BigInteger startBufferAddress = fContentProvider.getBufferTopAddress();
		startBufferAddress = MemoryViewUtil.alignToBoundary(startBufferAddress, getAddressableUnitPerLine());
		
		if (startAddress.compareTo(startBufferAddress) == 0)
			return true;
		
		return false;
	}
	
	private boolean isAtBottomLimit()
	{
		BigInteger endAddress = fContentInput.getEndAddress();
		endAddress = MemoryViewUtil.alignToBoundary(endAddress, getAddressableUnitPerLine());
		
		BigInteger endBufferAddress = fContentProvider.getBufferEndAddress();
		endBufferAddress = MemoryViewUtil.alignToBoundary(endBufferAddress, getAddressableUnitPerLine());
		
		if (endAddress.compareTo(endBufferAddress) == 0)
			return true;
		
		return false;		
	}
	
	private boolean needMoreLines()
	{
		if (getMemoryBlock() instanceof IMemoryBlockExtension)
		{		
			Table table = fTableViewer.getTable();
			TableItem firstItem = table.getItem(0);
			TableItem lastItem = table.getItem(table.getItemCount()-1);
			
			if (firstItem == null || lastItem == null)
				return true;
			
			TableRenderingLine first = (TableRenderingLine)firstItem.getData();
			TableRenderingLine last = (TableRenderingLine) lastItem.getData();
			
			if (first == null ||last == null)
			{
				// For some reason, the table does not return the correct number
				// of table items in table.getItemCount(), causing last to be null.
				// This check is to ensure that we don't get a null pointer exception.
				return true;
			}
			
			BigInteger startAddress = new BigInteger(first.getAddress(), 16);
			BigInteger lastAddress = new BigInteger(last.getAddress(), 16);
			int addressableUnit = getAddressableUnitPerLine();
			lastAddress = lastAddress.add(BigInteger.valueOf(addressableUnit));
			
			BigInteger topVisibleAddress = getTopVisibleAddress();
			long numVisibleLines = getNumberOfVisibleLines();
			long numOfBytes = numVisibleLines * addressableUnit;
			
			BigInteger lastVisibleAddrss = topVisibleAddress.add(BigInteger.valueOf(numOfBytes));
			
			// if there are only 3 lines left at the top, refresh
			BigInteger numTopLine = topVisibleAddress.subtract(startAddress).divide(BigInteger.valueOf(addressableUnit));
			if (numTopLine.compareTo(BigInteger.valueOf(3)) <= 0 && (startAddress.compareTo(BigInteger.valueOf(0)) != 0))
			{
				if (!isAtTopLimit())
					return true;
			}
			
			// if there are only 3 lines left at the bottom, refresh
			BigInteger numBottomLine = lastAddress.subtract(lastVisibleAddrss).divide(BigInteger.valueOf(addressableUnit));
			if (numBottomLine.compareTo(BigInteger.valueOf(3)) <= 0)
			{
				if (!isAtBottomLimit())
					return true;
			}
			
			return false;
		}
		
		return false;
	}

	private void handleTableMouseEvent(MouseEvent e) {
		// figure out new cursor position based on here the mouse is pointing
		TableItem[] tableItems = fTableViewer.getTable().getItems();
		TableItem selectedRow = null;
		int colNum = -1;
		int numCol = fTableViewer.getColumnProperties().length;
		
		for (int j=0; j<tableItems.length; j++)
		{
			TableItem item = tableItems[j];
			for (int i=0; i<numCol; i++)
			{
				Rectangle bound = item.getBounds(i);
				if (bound.contains(e.x, e.y))
				{
					colNum = i;
					selectedRow = item;
					break;
				}
			}
		}
		
		// if column position cannot be determined, return
		if (colNum < 1)
			return;
		
		// handle user mouse click onto table
		// move cursor to new position
		if (selectedRow != null)
		{
			int row = fTableViewer.getTable().indexOf(selectedRow);
			fTableCursor.setVisible(true);
			fTableCursor.setSelection(row, colNum);
			
			// manually call this since we don't get an event when
			// the table cursor changes selection.
			handleCursorMoved();
			
			fTableCursor.setFocus();
		}			
	}
	
	/**
	 * Handle column size changed event from synchronizer
	 * @param newColumnSize
	 */
	private void columnSizeChanged(final int newColumnSize) {
		// ignore event if view tab is disabled
		if (!isVisible())
			return;

		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				format(getBytesPerLine(), newColumnSize);
			}
		});
	}
	
	/**
	 * @param newRowSize - new row size in number of bytes
	 */
	private void rowSizeChanged(final int newRowSize)
	{
		// ignore event if view tab is disabled
		if (!isVisible())
			return;
		
		int bytesPerLine = newRowSize;
		int col = getBytesPerColumn();
		if (bytesPerLine < getBytesPerColumn())
			col = bytesPerLine;

		final int columnSize = col;
		final int rowSize = bytesPerLine;
		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				format(rowSize, columnSize);
			}
		});		
	}
	
	private void handleCursorMouseEvent(MouseEvent e){
		if (e.button == 1)
		{
			int col = fTableCursor.getColumn();
			if (col > 0 && col <= (getNumCol()))
				activateCellEditor(null);
		}			
	}
	
	/**
	 * Activate cell editor and pre-fill it with initial value.
	 * If initialValue is null, use cell content as initial value
	 * @param initialValue
	 */
	private void activateCellEditor(String initialValue) {
		
		int col = fTableCursor.getColumn();
		int row = findAddressIndex(fSelectedAddress);
		
		if (row < 0)
			return;
		// do not allow user to edit address column
		if (col == 0 || col > getNumCol())
		{
			return;
		}
		
		ICellModifier cellModifier = null;
		
		if (fTableViewer == null)
		{
			return;
		}
		cellModifier = fTableViewer.getCellModifier();
		
		TableItem tableItem = fTableViewer.getTable().getItem(row);
		
		Object element = tableItem.getData();
		Object property = fTableViewer.getColumnProperties()[col];
		Object value = cellModifier.getValue(element, (String)property);
		
		// The cell modifier canModify function always returns false if the edit action 
		// is not invoked from here.  This is to prevent data to be modified when
		// the table cursor loses focus from a cell.  By default, data will
		// be changed in a table when the cell loses focus.  This is to workaround
		// this default behavior and only change data when the cell editor
		// is activated.
		((TableRenderingCellModifier)cellModifier).setEditActionInvoked(true);
		boolean canEdit = cellModifier.canModify(element, (String)property);
		((TableRenderingCellModifier)cellModifier).setEditActionInvoked(false);
		
		if (!canEdit)
			return;
		
		// activate based on current cursor position
		TextCellEditor selectedEditor = (TextCellEditor)fTableViewer.getCellEditors()[col];

		
		if (fTableViewer != null && selectedEditor != null)
		{
			// The control that will be the editor must be a child of the Table
			Text text = (Text)selectedEditor.getControl();
			
			String cellValue  = null;
			
			if (initialValue != null)
			{
				cellValue = initialValue;	
			}
			else	
			{
				cellValue = ((String)value);
			}
			
			text.setText(cellValue);
	
			fCursorEditor.horizontalAlignment = SWT.LEFT;
			fCursorEditor.grabHorizontal = true;
	
			// Open the text editor in selected column of the selected row.
			fCursorEditor.setEditor (text, tableItem, col);
	
			// Assign focus to the text control
			selectedEditor.setFocus();
			
			if (initialValue != null)
			{
				text.clearSelection();
			}
			
			text.setFont(JFaceResources.getFont(IInternalDebugUIConstants.FONT_NAME));

			// add listeners for the text control
			addListeners(text);
			
			// move cursor below text control
			fTableCursor.moveBelow(text);
		}
	}
	
	/**
	 * @param text
	 */
	private void addListeners(Text text) {
		fEditorFocusListener = new FocusAdapter() {
			public void focusLost(FocusEvent e)
			{
				handleTableEditorFocusLost(e);
			}
		};
		text.addFocusListener(fEditorFocusListener);
		
		fEditorKeyListener = new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				handleKeyEventInEditor(e);
			}
		};

		text.addKeyListener(fEditorKeyListener);
	}
	
	/**
	 * @param text
	 */
	private void removeListeners(Text text) {
		
		text.removeFocusListener(fEditorFocusListener);
		text.removeKeyListener(fEditorKeyListener);
	}
	
	private void handleTableEditorFocusLost(FocusEvent event)
	{
		final FocusEvent e = event;

		Display.getDefault().syncExec(new Runnable() {

			public void run()
			{
				try
				{
					int row = findAddressIndex(fSelectedAddress);
					int col = fTableCursor.getColumn();
					
					Text text = (Text)e.getSource();
					removeListeners(text);

					// get new value
					String newValue = text.getText();
					
					// modify memory at fRow and fCol
					modifyValue(row, col, newValue);
							
					// show cursor after modification is completed
					setCursorAtAddress(fSelectedAddress);
					fTableCursor.moveAbove(text);
					fTableCursor.setVisible(false);
					fTableCursor.setVisible(true);
				}
				catch (NumberFormatException e1)
				{
					MemoryViewUtil.openError(DebugUIMessages.MemoryViewCellModifier_failure_title, 
						DebugUIMessages.MemoryViewCellModifier_data_is_invalid, null);
				}		
			}
		});		
	}
	
	/**
	 * @param event
	 */
	private void handleKeyEventInEditor(KeyEvent event) {
		final KeyEvent e = event;
		Display.getDefault().asyncExec(new Runnable()
		{
			public void run()
			{
				Text text = (Text)e.getSource();
				int row = findAddressIndex(fSelectedAddress);
				int col = fTableCursor.getColumn();
				
				try
				{
					switch (e.keyCode)
					{
						case SWT.ARROW_UP :
							
							// move text editor box up one row		
							if (row-1 < 0)
								return;
						
							// modify value for current cell
							modifyValue(row, col, text.getText());
													
							row--;

							//	update cursor location and selection in table	
							fTableCursor.setSelection(row, col);
							handleCursorMoved();
							
							// remove listeners when focus is lost
							removeListeners(text);
							activateCellEditor(null);
							break;
						case SWT.ARROW_DOWN :
							
							// move text editor box down one row
							
							if (row+1 >= fTableViewer.getTable().getItemCount())
								return;
							
							// modify value for current cell
							modifyValue(row, col, text.getText());
						
							row++;
							
							//	update cursor location and selection in table								
							fTableCursor.setSelection(row, col);
							handleCursorMoved();
												
							// remove traverse listener when focus is lost
							removeListeners(text);
							activateCellEditor(null);		
							break;
						case 0:
							
						// if user has entered the max number of characters allowed in a cell, move to next cell
						// Extra changes will be used as initial value for the next cell
							int numCharsPerByte = getNumCharsPerByte();
							if (numCharsPerByte > 0)
							{
								if (text.getText().length() > getBytesPerColumn()*numCharsPerByte)
								{
									String newValue = text.getText();
									text.setText(newValue.substring(0, getBytesPerColumn()*numCharsPerByte));
									
									modifyValue(row, col, text.getText());
									
									// if cursor is at the end of a line, move to next line
									if (col >= getNumCol())
									{
										col = 1;
										row++;
									}
									else
									{
										// move to next column
										row++;
									}
									
									// update cursor position and selected address
									fTableCursor.setSelection(row, col);
									handleCursorMoved();
									
									removeListeners(text);
						
									// activate text editor at next cell
									activateCellEditor(newValue.substring(getBytesPerColumn()*numCharsPerByte));
								}
							}
							break;	
						case SWT.ESC:

							// if user has pressed escape, do not commit the changes
							// that's why "modifyValue" is not called
							fTableCursor.setSelection(row, col);
							handleCursorMoved();
					
							removeListeners(text);
							
							// cursor needs to have focus to remove focus from cell editor
							fTableCursor.setFocus();
							break;	
						default :
							numCharsPerByte = getNumCharsPerByte();
							if (numCharsPerByte > 0)
							{								
								if (text.getText().length()> getBytesPerColumn()* numCharsPerByte)
								{
									String newValue = text.getText();
									text.setText(newValue.substring(0,getBytesPerColumn()* numCharsPerByte));
									modifyValue(row, col, text.getText());
									// if cursor is at the end of a line, move to next line
									if (col >= getNumCol())
									{
										col = 1;
										row++;
									}
									else
									{
										col++;
									}
									
									fTableCursor.setSelection(row, col);
									handleCursorMoved();
									
									removeListeners(text);
									
									activateCellEditor(newValue.substring(getBytesPerColumn()*numCharsPerByte));
								}
							}
						break;
					}
				}
				catch (NumberFormatException e1)
				{
					MemoryViewUtil.openError(DebugUIMessages.MemoryViewCellModifier_failure_title, 
						DebugUIMessages.MemoryViewCellModifier_data_is_invalid, null);
					
					fTableCursor.setSelection(row, col);
					handleCursorMoved();
			
					removeListeners(text);
				}
			}
		});
	}

	
	/**
	 * Modify value and send new value to debug adapter
	 * @param row
	 * @param col
	 * @param newValue
	 * @throws NumberFormatException
	 */
	private void modifyValue(int row, int col, String newValue) throws NumberFormatException
	{
		if (newValue.length() == 0)
		{	
			// do not do anything if user has not entered anything
			return;
		}
		
		TableItem tableItem = fTableViewer.getTable().getItem(row);

		Object property = fTableViewer.getColumnProperties()[col];
		fTableViewer.getCellModifier().modify(tableItem, (String)property, newValue);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#becomesHidden()
	 */
	public void becomesHidden() {
		
		if (isVisible() == false)
		{
			// super should always be called
			super.becomesHidden();
			return;
		}

		super.becomesHidden();
		
		if (getMemoryBlock() instanceof IMemoryBlockExtension)
		{	
			updateRenderingLabel(false);
		}
			
		// once the view tab is disabled, all deltas information becomes invalid.
		// reset changed information and recompute if data has really changed when
		// user revisits the same tab.	
		fContentProvider.resetDeltas();
		
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#becomesVisible()
	 */
	public void becomesVisible() {
		
		// do not do anything if already visible
		if (isVisible() == true)
		{
			// super should always be called
			super.becomesVisible();
			return;
		}
		
		super.becomesVisible();
		
		boolean value = DebugUIPlugin.getDefault().getPreferenceStore().getBoolean(IDebugPreferenceConstants.PREF_DYNAMIC_LOAD_MEM);
		if (value != isDynamicLoad())
			// this call will cause a reload
			handleDyanicLoadChanged();
		else
			refresh();
		
		synchronize();
		updateRenderingLabel(true);
	}
	
	/**
	 * Resets this memory rendering.
	 * The cursor will be moved to the base address of the memory block.
	 * The table will be positioned to have the base address
	 * at the top.
	 * 
	 * @deprecated use <code>resetRendering</code> to reset this rendering.
	 */
	public void reset()
	{
		try {
			resetToBaseAddress();
		} catch (DebugException e) {
			MemoryViewUtil.openError(DebugUIMessages.AbstractTableRendering_12, DebugUIMessages.AbstractTableRendering_13, e); //
		}
	}
	
	/**
	 * Reset this rendering to the base address.  
	 * The cursor will be moved to the base address of the memory block.
	 * The table will be positioned to have the base address
	 * at the top.
	 * @throws DebugException
	 */
	private void resetToBaseAddress() throws DebugException
	{
		BigInteger baseAddress;

		if (getMemoryBlock() instanceof IMemoryBlockExtension)
		{
			baseAddress = ((IMemoryBlockExtension)getMemoryBlock()).getBigBaseAddress();
		}
		else
		{
			baseAddress = BigInteger.valueOf(getMemoryBlock().getStartAddress());
		}

		goToAddress(baseAddress);
		topVisibleAddressChanged(baseAddress, true);
	}
	
	/**
	 * Returns the currently selected address in this rendering.
	 * 
	 * @return the currently selected address in this rendering
	 */
	public BigInteger getSelectedAddress() {
		return fSelectedAddress;
	}

	/**
	 * Returns the currently selected content in this rendering as a String.
	 * 
	 * @return the currently selected content in this rendering
	 */
	public String getSelectedAsString() {

		if (isAddressOutOfRange(fSelectedAddress))
			return ""; //$NON-NLS-1$
		
		int col = fTableCursor.getColumn();
		TableItem rowItem = fTableCursor.getRow();
		int row = fTableViewer.getTable().indexOf(rowItem);
		
		if (col == 0)
		{
			return rowItem.getText(0);
		}
		
		// check precondition
		if (col > getBytesPerLine()/getBytesPerColumn())
		{
			return ""; //$NON-NLS-1$
		}
				
		TableItem tableItem = getTableViewer().getTable().getItem(row);
		
		return tableItem.getText(col);	
	}
	
	/**
	 * Returns the currently selected content in this rendering as MemoryByte.
	 * 
	 * @return the currently selected content in array of MemoryByte.  
	 * Returns an empty array if the selected address is out of buffered range.
	 */
	public MemoryByte[] getSelectedAsBytes()
	{
		if (isAddressOutOfRange(fSelectedAddress))
			return new MemoryByte[0];
		
		int col = fTableCursor.getColumn();
		TableItem rowItem = fTableCursor.getRow();
		
		// check precondition
		if (col == 0 || col > getBytesPerLine()/getBytesPerColumn())
		{
			return new MemoryByte[0];
		}
		
		Object data = rowItem.getData();
		if (data == null || !(data instanceof TableRenderingLine))
			return new MemoryByte[0];
		
		TableRenderingLine line = (TableRenderingLine)data;
		int offset = (col-1)*(getAddressableUnitPerColumn()*getAddressableSize());
		int end = offset + (getAddressableUnitPerColumn()*getAddressableSize());
		
		// make a copy of the bytes to ensure that data cannot be changed
		// by caller
		MemoryByte[] bytes = line.getBytes(offset, end);
		MemoryByte[] retBytes = new MemoryByte[bytes.length];
		
		System.arraycopy(bytes, 0, retBytes, 0, bytes.length);
		
		return retBytes;
	}
	
	/**
	 * Returns the number of characters a byte will convert to
	 * or -1 if unknown.
	 * 
	 * @return the number of characters a byte will convert to
	 *  or -1 if unknown
	 */
	public int getNumCharsPerByte()
	{
		return -1;
	}
	
	private int getMinTableItemHeight(Table table){
		
		// Hack to get around Linux GTK problem.
		// On Linux GTK, table items have variable item height as
		// carriage returns are actually shown in a cell.  Some rows will be
		// taller than others.  When calculating number of visible lines, we
		// need to find the smallest table item height.  Otherwise, the rendering
		// underestimates the number of visible lines.  As a result the rendering
		// will not be able to get more memory as needed.
		if (MemoryViewUtil.isLinuxGTK())
		{
			// check each of the items and find the minimum
			TableItem[] items = table.getItems();
			int minHeight = table.getItemHeight();
			for (int i=0; i<items.length; i++)
			{
				minHeight = Math.min(items[i].getBounds(0).height, minHeight);
			}
			
			return minHeight;
				
		}
		return table.getItemHeight();
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.core.runtime.IAdaptable#getAdapter(java.lang.Class)
	 */
	public Object getAdapter(Class adapter) {
		
		if (adapter == IColorProvider.class)
			return getColorProviderAdapter();
		
		if (adapter == ILabelProvider.class)
			return getLabelProviderAdapter();
		
		if (adapter == IFontProvider.class)
			return getFontProviderAdapter();
		
		if (adapter == IMemoryBlockTablePresentation.class)
			return getTablePresentationAdapter();
		
		if (adapter == IWorkbenchAdapter.class)
		{
			// needed workbench adapter to fill the title of property page
			if (fWorkbenchAdapter == null) {
				fWorkbenchAdapter = new IWorkbenchAdapter() {
					public Object[] getChildren(Object o) {
						return new Object[0];
					}
	
					public ImageDescriptor getImageDescriptor(Object object) {
						return null;
					}
	
					public String getLabel(Object o) {
						return AbstractTableRendering.this.getLabel();
					}
	
					public Object getParent(Object o) {
						return null;
					}
				};
			}
			return fWorkbenchAdapter;
		}
		
		if (adapter == IMemoryBlockConnection.class) {
			if (fConnection == null) {
				fConnection = new IMemoryBlockConnection() {
					public void update() {
						try {
							fContentProvider.takeContentSnapshot();
							if (getMemoryBlock() instanceof IMemoryBlockExtension)
							{
								BigInteger address = ((IMemoryBlockExtension)getMemoryBlock()).getBigBaseAddress();
								if (address.compareTo(fContentProvider.getContentBaseAddress()) != 0)
								{
									// get to new address
									setSelectedAddress(address);
									updateSyncSelectedAddress();
									fTopRowAddress = address;
									fContentInput.updateContentBaseAddress();
									fContentInput.setLoadAddress(address);
								}
								fContentProvider.loadContentForExtendedMemoryBlock();
							}
							else
								fContentProvider.loadContentForSimpleMemoryBlock();
	
							// update UI asynchronously
							Display display = DebugUIPlugin.getDefault().getWorkbench().getDisplay();
							display.asyncExec(new Runnable() {
								public void run() {
									updateLabels();
									
									if (getMemoryBlock() instanceof IMemoryBlockExtension) {
										int topIdx = findAddressIndex(fTopRowAddress);
										if (topIdx != -1) {
											setTopIndex(fTableViewer.getTable(),topIdx);
										}
									}
									
									// cursor needs to be refreshed after reload
									if (isAddressVisible(fSelectedAddress))
									{
										setCursorAtAddress(fSelectedAddress);
										fTableCursor.setVisible(true);
										fTableCursor.redraw();
									}
									else
									{
										fTableCursor.setVisible(false);
									}
									
									if (!isDynamicLoad())
										updateSyncPageStartAddress();
									
									updateSyncTopAddress();
								}
							});
						} catch (DebugException e) {
							displayError(e);
						}
					}
				};
			}
			return fConnection;
		}	
		
		return super.getAdapter(adapter);
	}
	
	private boolean hasCustomizedDecorations()
	{
		if (getFontProviderAdapter() == null &&
			getColorProviderAdapter() == null &&
			getLabelProviderAdapter() == null)
			return false;
		return true;
	}
	
	private boolean isBaseAddressChanged()
	{
		try {
			IMemoryBlock mb = getMemoryBlock();
			if (mb instanceof IMemoryBlockExtension)
			{
				BigInteger baseAddress = ((IMemoryBlockExtension)mb).getBigBaseAddress();
				if (baseAddress != null)
				{
					if (!baseAddress.equals(fContentInput.getContentBaseAddress()))
						return true;
				}
			}
		} catch (DebugException e1) {
			return false;
		}
		return false;
	}
	
	/**
	 * Returns the color provider for this rendering's memory block or
	 * <code>null</code> if none.
	 * <p>
	 * By default a color provider is obtained by asking this rendering's
	 * memory block for its {@link IColorProvider} adapter. When the color
	 * provider is queried for color information, it is provided with a
	 * {@link MemoryRenderingElement} as an argument. 
	 * </p>
	 * @return the color provider for this rendering's memory block,
	 *  or <code>null</code>
	 */
	protected IColorProvider getColorProviderAdapter()
	{
		return (IColorProvider)getMemoryBlock().getAdapter(IColorProvider.class);
	}
	
	/**
	 * Returns the label provider for this rendering's memory block or
	 * <code>null</code> if none.
	 * <p>
	 * By default a label provider is obtained by asking this rendering's
	 * memory block for its {@link ILabelProvider} adapter. When the label
	 * provider is queried for label information, it is provided with a
	 * {@link MemoryRenderingElement} as an argument. 
	 * </p>
	 * @return the label provider for this rendering's memory block,
	 *  or <code>null</code>
	 */
	protected ILabelProvider getLabelProviderAdapter()
	{
		return (ILabelProvider)getMemoryBlock().getAdapter(ILabelProvider.class);
	}
	
	/**
	 * Returns the font provider for this rendering's memory block or
	 * <code>null</code> if none.
	 * <p>
	 * By default a font provider is obtained by asking this rendering's
	 * memory block for its {@link IFontProvider} adapter. When the font
	 * provider is queried for font information, it is provided with a
	 * {@link MemoryRenderingElement} as an argument. 
	 * </p>
	 * @return the font provider for this rendering's memory block,
	 *  or <code>null</code>
	 */
	protected IFontProvider getFontProviderAdapter()
	{
		return (IFontProvider)getMemoryBlock().getAdapter(IFontProvider.class);
	}
	
	/**
	 * Returns the table presentation for this rendering's memory block or
	 * <code>null</code> if none.
	 * <p>
	 * By default a table presentation is obtained by asking this rendering's
	 * memory block for its {@link IMemoryBlockTablePresentation} adapter.
	 * </p>
	 * @return the table presentation for this rendering's memory block,
	 *  or <code>null</code>
	 */
	protected IMemoryBlockTablePresentation getTablePresentationAdapter()
	{
		return (IMemoryBlockTablePresentation)getMemoryBlock().getAdapter(IMemoryBlockTablePresentation.class);
	}
	
	private boolean isDynamicLoad()
	{
		return fContentProvider.isDynamicLoad();
	}
	
	private int getPageSizeInUnits()
	{
		return fPageSize * getAddressableUnitPerLine();
	}
	
	private void setSelectedAddress(BigInteger address)
	{
		fSelectedAddress = address;
	}
		
	/**
	 * Setup the viewer so it supports hovers to show the offset of each field
	 */
	private void createToolTip() {
		
		fToolTipShell = new Shell(DebugUIPlugin.getShell(), SWT.ON_TOP | SWT.RESIZE );
		GridLayout gridLayout = new GridLayout();
		gridLayout.numColumns = 1;
		gridLayout.marginWidth = 2;
		gridLayout.marginHeight = 0;
		fToolTipShell.setLayout(gridLayout);
		fToolTipShell.setBackground(fTableViewer.getTable().getDisplay().getSystemColor(SWT.COLOR_INFO_BACKGROUND));
		
		final Control toolTipControl = createToolTipControl(fToolTipShell);
		
		if (toolTipControl == null)
		{
			// if client decide not to use tooltip support
			fToolTipShell.dispose();
			return;
		}
		
		MouseTrackAdapter listener = new MouseTrackAdapter(){
			
			private TableItem fTooltipItem = null;
			private int fCol = -1;
			
			public void mouseExit(MouseEvent e){
				
				if (!fToolTipShell.isDisposed())
					fToolTipShell.setVisible(false);
				fTooltipItem = null;
			}
			
			public void mouseHover(MouseEvent e){
				
				Point hoverPoint = new Point(e.x, e.y);
				Control control = null;
				
				if (e.widget instanceof Control)
					control = (Control)e.widget;
				
				if (control == null)
					return;
				
				hoverPoint = control.toDisplay(hoverPoint);
				TableItem item = getItem(hoverPoint);
				int column = getColumn(hoverPoint);
				
				//Only if there is a change in hover
				if(this.fTooltipItem != item || fCol != column){
					
					//Keep Track of the latest hover
					fTooltipItem = item;
					fCol = column;
					
					if(item != null){
						toolTipAboutToShow(toolTipControl, fTooltipItem, column);
						
						//Setting location of the tooltip
						Rectangle shellBounds = fToolTipShell.getBounds();
						shellBounds.x = hoverPoint.x;
						shellBounds.y = hoverPoint.y + item.getBounds(0).height;
						
						fToolTipShell.setBounds(shellBounds);
						fToolTipShell.pack();
						
						fToolTipShell.setVisible(true);
					}
					else {
						fToolTipShell.setVisible(false);
					}
				}
			}
		};
		
		fTableViewer.getTable().addMouseTrackListener(listener);
		fTableCursor.addMouseTrackListener(listener);
	}
	
	/**
	 * Bug with table widget,BUG 113015, the widget is not able to return the correct
	 * table item if SWT.FULL_SELECTION is not on when the table is created.
	 * Created the following function to work around the problem.
	 * We can remove this method when the bug is fixed.
	 * @param point
	 * @return the table item where the point is located, return null if the item cannot be located.
	 */
	private TableItem getItem(Point point)
	{
		TableItem[] items = fTableViewer.getTable().getItems();
		for (int i=0; i<items.length; i++)
		{
			Point start = new Point(items[i].getBounds(0).x, items[i].getBounds(0).y);
			start = fTableViewer.getTable().toDisplay(start);
			Point end = new Point(start.x + items[i].getBounds(0).width, start.y + items[i].getBounds(0).height);
			
			if (start.y < point.y && point.y < end.y)
				return items[i];
		}
		return null;
	}
	
	/**
	 * Method for figuring out which column the point is located.
	 * @param point
	 * @return the column index where the point is located, return -1 if column is not found.
	 */
	private int getColumn(Point point)
	{
		int colCnt = fTableViewer.getTable().getColumnCount();
		TableItem item = fTableViewer.getTable().getItem(0);
		for (int i=0; i<colCnt; i++)
		{
			Point start = new Point(item.getBounds(i).x, item.getBounds(i).y);
			start = fTableViewer.getTable().toDisplay(start);
			Point end = new Point(start.x + item.getBounds(i).width, start.y + item.getBounds(i).height);
			
			if (start.x < point.x && end.x > point.x)
				return i;
		}
		return -1;
	}

	/**
	 * Creates the control used to display tool tips for cells in this table. By default
	 * a label is used to display the address of the cell. Clients may override this
	 * method to create custom tooltip controls.
	 * <p>
	 * Also see the methods <code>getToolTipText(...)</code> and 
	 * <code>toolTipAboutToShow(...)</code>.
	 * </p>
	 * @param composite parent for the tooltip control
	 * @return the tooltip control to be displayed
	 * @since 3.2
	 */
	protected Control createToolTipControl(Composite composite) {
		Control fToolTipLabel = new Label(composite, SWT.NONE);
		fToolTipLabel.setForeground(fTableViewer.getTable().getDisplay().getSystemColor(SWT.COLOR_INFO_FOREGROUND));
		fToolTipLabel.setBackground(fTableViewer.getTable().getDisplay().getSystemColor(SWT.COLOR_INFO_BACKGROUND));
		fToolTipLabel.setLayoutData(new GridData(GridData.FILL_HORIZONTAL |
				GridData.VERTICAL_ALIGN_CENTER));
		return fToolTipLabel;
	}
	
	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IResettableMemoryRendering#resetRendering()
	 */
	public void resetRendering() throws DebugException {
		resetToBaseAddress();
	}

	/**
	 * Called when the tool tip is about to show in this rendering.
	 * Clients who overrides <code>createTooltipControl</code> may need to
	 * also override this method to ensure that the tooltip shows up properly
	 * in their customized control.
	 * <p>
	 * By default a text tooltip is displayed, and the contents for the tooltip
	 * are generated by the <code>getToolTipText(...)</code> method.
	 * </p>
	 * @param toolTipControl - the control for displaying the tooltip
	 * @param item - the table item where the mouse is pointing.
	 * @param col - the column at which the mouse is pointing.
	 * @since 3.2
	 */
	protected void toolTipAboutToShow(Control toolTipControl, TableItem item,
			int col) {
		if (toolTipControl instanceof Label) {
			BigInteger address = getAddressFromTableItem(item, col);
			if (address != null) {
				Object data = item.getData();
				if (data instanceof TableRenderingLine) {
					TableRenderingLine line = (TableRenderingLine) data;

					if (col > 0) {
						int start = (col - 1) * getBytesPerColumn();
						int end = start + getBytesPerColumn();
						MemoryByte[] bytes = line.getBytes(start, end);

						String str = getToolTipText(address, bytes);

						if (str != null)
							((Label) toolTipControl).setText(str);
					} else {
						String str = getToolTipText(address,
								new MemoryByte[] {});

						if (str != null)
							((Label) toolTipControl).setText(str);
					}
				}
			}
		}
	}
	
	/**
	 * Returns the text to display in a tool tip at the specified address
	 * for the specified bytes. By default the address of the bytes is displayed.
	 * Subclasses may override.
	 * 
	 * @param address address of cell that tool tip is displayed for 
	 * @param bytes the bytes in the cell
	 * @return the tooltip text for the memory bytes located at the specified
	 *         address
	 * @since 3.2
	 */
	protected String getToolTipText(BigInteger address, MemoryByte[] bytes)
	{
		StringBuffer buf = new StringBuffer("0x"); //$NON-NLS-1$
		buf.append(address.toString(16).toUpperCase());
		
		return buf.toString();
	}
	
	
	private String getRowPrefId(String modelId) {
		String rowPrefId = IDebugPreferenceConstants.PREF_ROW_SIZE + ":" + modelId; //$NON-NLS-1$
		return rowPrefId;
	}

	private String getColumnPrefId(String modelId) {
		String colPrefId = IDebugPreferenceConstants.PREF_COLUMN_SIZE + ":" + modelId; //$NON-NLS-1$
		return colPrefId;
	}
	
	/**
	 * @param modelId
	 * @return default number of addressable units per line for the model
	 */
	private int getDefaultRowSizeByModel(String modelId)
	{
		int row = DebugUITools.getPreferenceStore().getInt(getRowPrefId(modelId));
		if (row == 0)
		{
			DebugUITools.getPreferenceStore().setValue(getRowPrefId(modelId), IDebugPreferenceConstants.PREF_ROW_SIZE_DEFAULT);
		}
		
		row = DebugUITools.getPreferenceStore().getInt(getRowPrefId(modelId));
		return row;
		
	}
	
	/**
	 * @param modelId
	 * @return default number of addressable units per column for the model
	 */
	private int getDefaultColumnSizeByModel(String modelId)
	{
		int col = DebugUITools.getPreferenceStore().getInt(getColumnPrefId(modelId));
		if (col == 0)
		{
			DebugUITools.getPreferenceStore().setValue(getColumnPrefId(modelId), IDebugPreferenceConstants.PREF_COLUMN_SIZE_DEFAULT);
		}
		
		col = DebugUITools.getPreferenceStore().getInt(getColumnPrefId(modelId));
		return col;
	}

	
	/**
	 * Returns text for the given memory bytes at the specified address for the specified
	 * rendering type. This is called by the label provider for.
	 * Subclasses must override.
	 * 
	 * @param renderingTypeId rendering type identifier
	 * @param address address where the bytes belong to
	 * @param data the bytes
	 * @return a string to represent the memory. Cannot not return <code>null</code>.
	 * 	Returns a string to pad the cell if the memory cannot be converted
	 *  successfully.
	 */
	abstract public String getString(String renderingTypeId, BigInteger address, MemoryByte[] data);
	
	/**
	 * Returns bytes for the given text corresponding to bytes at the given
	 * address for the specified rendering type. This is called by the cell modifier
	 * when modifying bytes in a memory block.
	 * Subclasses must convert the string value to an array of bytes.  The bytes will
	 * be passed to the debug adapter for memory block modification.
	 * Returns <code>null</code> if the bytes cannot be formatted properly.
	 * 
	 * @param renderingTypeId rendering type identifier
	 * @param address address the bytes begin at
	 * @param currentValues current values of the data in bytes format
	 * @param newValue the string to be converted to bytes
	 * @return the bytes converted from a string
	 */
	abstract public byte[] getBytes(String renderingTypeId, BigInteger address, MemoryByte[] currentValues, String newValue);


}	

