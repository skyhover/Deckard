/*******************************************************************************
 * Copyright (c) 2006 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 * 
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/

package org.eclipse.debug.internal.ui.memory.provisional;


import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.Command;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.core.runtime.jobs.ISchedulingRule;
import org.eclipse.core.runtime.jobs.Job;
import org.eclipse.debug.core.DebugException;
import org.eclipse.debug.core.model.IMemoryBlock;
import org.eclipse.debug.core.model.IMemoryBlockExtension;
import org.eclipse.debug.core.model.MemoryByte;
import org.eclipse.debug.internal.ui.DebugUIMessages;
import org.eclipse.debug.internal.ui.DebugUIPlugin;
import org.eclipse.debug.internal.ui.IInternalDebugUIConstants;
import org.eclipse.debug.internal.ui.memory.IPersistableDebugElement;
import org.eclipse.debug.internal.ui.preferences.IDebugPreferenceConstants;
import org.eclipse.debug.internal.ui.viewers.model.provisional.IModelChangedListener;
import org.eclipse.debug.internal.ui.viewers.model.provisional.IModelDelta;
import org.eclipse.debug.internal.ui.viewers.model.provisional.IModelProxy;
import org.eclipse.debug.internal.ui.viewers.model.provisional.IStatusMonitor;
import org.eclipse.debug.internal.ui.views.memory.MemoryViewUtil;
import org.eclipse.debug.internal.ui.views.memory.renderings.AbstractBaseTableRendering;
import org.eclipse.debug.internal.ui.views.memory.renderings.AbstractVirtualContentTableModel;
import org.eclipse.debug.internal.ui.views.memory.renderings.AsyncCopyTableRenderingAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.AsyncPrintTableRenderingAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.AsyncTableRenderingCellModifier;
import org.eclipse.debug.internal.ui.views.memory.renderings.AsyncTableRenderingViewer;
import org.eclipse.debug.internal.ui.views.memory.renderings.AsyncVirtualContentTableViewer;
import org.eclipse.debug.internal.ui.views.memory.renderings.CopyTableRenderingToClipboardAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.FormatTableRenderingAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.FormatTableRenderingDialog;
import org.eclipse.debug.internal.ui.views.memory.renderings.GoToAddressAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.GoToAddressComposite;
import org.eclipse.debug.internal.ui.views.memory.renderings.IPresentationErrorListener;
import org.eclipse.debug.internal.ui.views.memory.renderings.IVirtualContentListener;
import org.eclipse.debug.internal.ui.views.memory.renderings.MemorySegment;
import org.eclipse.debug.internal.ui.views.memory.renderings.PendingPropertyChanges;
import org.eclipse.debug.internal.ui.views.memory.renderings.PrintTableRenderingAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.ReformatAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.ResetToBaseAddressAction;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingContentDescriptor;
import org.eclipse.debug.internal.ui.views.memory.renderings.TableRenderingLine;
import org.eclipse.debug.ui.DebugUITools;
import org.eclipse.debug.ui.IDebugUIConstants;
import org.eclipse.debug.ui.memory.AbstractTableRendering;
import org.eclipse.debug.ui.memory.IMemoryBlockTablePresentation;
import org.eclipse.debug.ui.memory.IMemoryRendering;
import org.eclipse.debug.ui.memory.IMemoryRenderingContainer;
import org.eclipse.debug.ui.memory.IMemoryRenderingSite;
import org.eclipse.debug.ui.memory.IMemoryRenderingSynchronizationService;
import org.eclipse.debug.ui.memory.IMemoryRenderingType;
import org.eclipse.debug.ui.memory.IResettableMemoryRendering;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.action.Separator;
import org.eclipse.jface.dialogs.IDialogConstants;
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
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredViewer;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ControlListener;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseTrackAdapter;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.commands.ICommandService;
import org.eclipse.ui.contexts.IContextService;
import org.eclipse.ui.dialogs.PropertyDialogAction;
import org.eclipse.ui.model.IWorkbenchAdapter;
import org.eclipse.ui.part.PageBook;
import org.eclipse.ui.progress.UIJob;

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

 * @since 3.2
 */
public abstract class AbstractAsyncTableRendering extends AbstractBaseTableRendering implements IPropertyChangeListener, IResettableMemoryRendering {

	/**
	 *  Property identifier for the selected address in a table rendering
	 *  This property is used for synchronization between renderings.
	 */
	public static final String PROPERTY_SELECTED_ADDRESS = AbstractTableRendering.PROPERTY_SELECTED_ADDRESS;
	
	/**
	 * Property identifier for the column size in a table rendering
	 * This property is used for synchronization between renderings.
	 */
	public static final String PROPERTY_COL_SIZE = AbstractTableRendering.PROPERTY_COL_SIZE;
	
	/**
	 * Property identifier for the top row address in a table rendering. 
	 * This property is used for synchronization between renderings.
	 */
	public static final String PROPERTY_TOP_ADDRESS = AbstractTableRendering.PROPERTY_TOP_ADDRESS;
	
	private static final String ID_ASYNC_TABLE_RENDERING_CONTEXT = "org.eclipse.debug.ui.memory.abstractasynctablerendering"; //$NON-NLS-1$
	private static final String ID_GO_TO_ADDRESS_COMMAND = "org.eclipse.debug.ui.command.gotoaddress"; //$NON-NLS-1$
	
	/**
	 * Property identifier for the row size in a table rendering
	 * This property is used for synchronization between renderings.
	 */
	public static final String PROPERTY_ROW_SIZE = AbstractTableRendering.PROPERTY_ROW_SIZE;
	
	private boolean fActivated = false;
	
//	TODO:  review use of MemorySegment, need to be careful to ensure flexible hierarchy
	
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
			if (!fIsShowAddressColumn)
			{
				fTableViewer.getTable().getColumn(0).setWidth(0);
			}
			else
			{
				fTableViewer.getTable().getColumn(0).pack();
			}
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
			BigInteger address = fContentDescriptor.getLoadAddress();
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
			BigInteger address = fContentDescriptor.getLoadAddress();
			address = address.subtract(BigInteger.valueOf(getPageSizeInUnits()));
			handlePageStartAddressChanged(address);
		}
	}
	
	private class RenderingGoToAddressAction extends GoToAddressAction
	{
		public RenderingGoToAddressAction(AbstractBaseTableRendering rendering) {
			super(rendering);
		}
		
		public void run() {
			showGoToAddressComposite();
		}
	}
	
	private class SwitchPageJob extends UIJob {
		private Object fLock = new Object();
		private boolean fShowMessagePage = false;
		private String fMessage = ""; //$NON-NLS-1$

		private SwitchPageJob() {
			super("SwitchPageJob");//$NON-NLS-1$
			setSystem(true);
		}

		private void setShowMessagePage(boolean showMsg) {
			synchronized(fLock)
			{
				fShowMessagePage = showMsg;
			}
		}

		private void setMessage(String message) {
			synchronized(fLock)
			{
				fMessage = message;
			}
		}

		public IStatus runInUIThread(IProgressMonitor monitor) {
			
			if (fPageBook.isDisposed())
				return Status.OK_STATUS;
			
			String msgToShow = null;
			boolean showMsgPage = false;
			synchronized (fLock) {
				msgToShow = fMessage;
				showMsgPage = fShowMessagePage;
			}
			
			if (showMsgPage) {
				StyledText styleText = null;
				fShowMessage = true;

				styleText = fTextViewer.getTextWidget();

				if (styleText != null)
					styleText.setText(msgToShow);
				fPageBook.showPage(fTextViewer.getControl());
			} else {
				fShowMessage = false;
				fPageBook.showPage(fTableViewer.getControl().getParent());
			}
			return Status.OK_STATUS;
		}
	}
	
	
	private class SerialByObjectRule implements ISchedulingRule
	{
    	private Object fObject = null;
    	
    	public SerialByObjectRule(Object lock) {
    		fObject = lock;
    	}

		/* (non-Javadoc)
		 * @see org.eclipse.core.runtime.jobs.ISchedulingRule#contains(org.eclipse.core.runtime.jobs.ISchedulingRule)
		 */
		public boolean contains(ISchedulingRule rule) {
			return rule == this;
		}

		/* (non-Javadoc)
		 * @see org.eclipse.core.runtime.jobs.ISchedulingRule#isConflicting(org.eclipse.core.runtime.jobs.ISchedulingRule)
		 */
		public boolean isConflicting(ISchedulingRule rule) {
			if (rule instanceof SerialByObjectRule) {
				SerialByObjectRule rRule = (SerialByObjectRule) rule;
				return fObject == rRule.fObject;
			}
			return false;
		}
	}
	
	private PageBook fPageBook;
	private AsyncTableRenderingViewer fTableViewer;
	private TextViewer fTextViewer;
	private Shell fToolTipShell;
	private MemoryViewPresentationContext fPresentationContext;
	private int fAddressableSize;
	private TableRenderingContentDescriptor fContentDescriptor;
	private int fBytePerLine;
	private int fColumnSize;
	private boolean fShowMessage = false;
	private String fLabel;
	private IWorkbenchAdapter fWorkbenchAdapter;
	private int fPageSize;
	private SashForm fSashForm;
	private GoToAddressComposite fGoToAddressComposite;
	
	// actions
	private GoToAddressAction fGoToAddressAction;
	private PrintTableRenderingAction fPrintViewTabAction;
	private CopyTableRenderingToClipboardAction fCopyToClipboardAction;
	private FormatTableRenderingAction fFormatRenderingAction;
	private ReformatAction fReformatAction;
	private ToggleAddressColumnAction fToggleAddressColumnAction;
	private ResetToBaseAddressAction fResetMemoryBlockAction;
	private PropertyDialogAction fPropertiesDialogAction;
	private NextPageAction fNextAction;
	private PrevPageAction fPrevAction;
	
	private ArrayList fContext = new ArrayList();
	private AbstractHandler fGoToAddressHandler;
	
	private boolean fIsCreated = false;
	private boolean fIsDisposed = false;
	private boolean fIsShowAddressColumn = true;
	
	private SwitchPageJob fSwitchPageJob = new SwitchPageJob();
	private boolean fError = false;
	
	private PendingPropertyChanges fPendingSyncProperties;
	
	// list of menu listeners for popupMenuMgr.  
	private ArrayList fMenuListeners;
	private MenuManager fMenuMgr;
	
	private ISchedulingRule serialByRenderingRule = new SerialByObjectRule(this);
	
	/** 
	 * Identifier for an empty group preceding all context menu actions 
	 * (value <code>"popUpBegin"</code>).
	 */
	public static final String EMPTY_MEMORY_GROUP = "popUpBegin"; //$NON-NLS-1$
	
	/**
	 * Identifier for an empty group following navigation actions in the rendering
	 * (value <code>navigationGroup</code>).
	 */
	public static final String EMPTY_NAVIGATION_GROUP = "navigationGroup"; //$NON-NLS-1$
	
	/**
	 * Identifier for an empty group following actions that are only applicable in
	 * non-auto loading mode
	 * (value <code>nonAutoLoadGroup</code>).
	 */
	public static final String EMPTY_NON_AUTO_LOAD_GROUP = "nonAutoLoadGroup"; //$NON-NLS-1$
	
	/**
	 * Identifier for an empty group following properties actions
	 * (value <code>propertyGroup</code>).
	 */
	public static final String EMPTY_PROPERTY_GROUP = "propertyGroup"; //$NON-NLS-1$
	
	private ISelectionChangedListener fViewerSelectionChangedListener = new ISelectionChangedListener() {
		public void selectionChanged(SelectionChangedEvent event) {
			updateSyncTopAddress(getTopVisibleAddress());
			updateSyncSelectedAddress(getSelectedAddress());
		}
	};
	
	private SelectionAdapter fScrollBarSelectionListener = new SelectionAdapter() {
		public void widgetSelected(SelectionEvent e) {
			updateSyncTopAddress(getTopVisibleAddress());
		}
	};
	
	private IModelChangedListener fModelChangedListener = new IModelChangedListener() {
		public void modelChanged(IModelDelta delta, IModelProxy proxy) {
			if (delta.getElement() == getMemoryBlock())
			{
				showTable();
				updateRenderingLabel(isVisible());
			}
		}};
	
	private IVirtualContentListener fViewerListener = new IVirtualContentListener() {

		public void handledAtBufferStart() {
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
			{
				if (isDynamicLoad())
				{
					BigInteger address = getTopVisibleAddress();
					if (address != null && !isAtTopLimit())
						reloadTable(address);
				}
			}
		}

		public void handleAtBufferEnd() {
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
			{
				if (isDynamicLoad())
				{
					BigInteger address = getTopVisibleAddress();
					if (address != null && !isAtBottomLimit())
						reloadTable(address);
				}
			}
		}

		public int getThreshold() {
			return 3;
		}};
		
	private IPresentationErrorListener fPresentationErrorListener = new IPresentationErrorListener() {
		public void handlePresentationFailure(IStatusMonitor monitor, IStatus status) {
			showMessage(status.getMessage());
		}};
	
	
	/**
	 * Constructs a new table rendering of the specified type.
	 * 
	 * @param renderingId memory rendering type identifier
	 */
	public AbstractAsyncTableRendering(String renderingId) {
		super(renderingId);
	}

	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IResettableMemoryRendering#resetRendering()
	 */
	public void resetRendering() throws DebugException {
		BigInteger baseAddress = fContentDescriptor.getContentBaseAddress();
		goToAddress(baseAddress);
		fTableViewer.setSelection(baseAddress);
		fTableViewer.setTopIndex(baseAddress);

		updateSyncTopAddress(baseAddress);		
		updateSyncSelectedAddress(baseAddress);
	}

	public Control createControl(Composite parent) {
		
		fPageBook = new PageBook(parent, SWT.NONE);
		createMessagePage(fPageBook);
		createTableViewer(fPageBook);
		addListeners();
		
		return fPageBook;
	}
	
	/**
	 * Create the error page of this rendering
	 * @param parent
	 */
	private void createMessagePage(Composite parent)
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
	 * @param parent
	 */
	private void createTableViewer(final Composite parent)
	{
		StringBuffer buffer = new StringBuffer();
		IMemoryRenderingType type = DebugUITools.getMemoryRenderingManager().getRenderingType(getRenderingId());
		buffer.append(type.getLabel());
		buffer.append(": "); //$NON-NLS-1$
		buffer.append(DebugUIMessages.AbstractAsyncTableRendering_2);
		
		Job job = new Job(buffer.toString()) {

			protected IStatus run(IProgressMonitor monitor) {
				
				// gather information from memory block
				initAddressableSize();					
				final BigInteger topVisibleAddress = getInitialTopVisibleAddress();
				BigInteger mbBaseAddress = null;
				try {
					mbBaseAddress = getMemoryBlockBaseAddress();
				} catch (DebugException e) {
					fError = true;
					showMessage(e.getMessage());
				}
				
				// if it takes too long to get the base address, and user has canceled
				// remove this rendering.
				if (monitor.isCanceled())
				{
					getMemoryRenderingContainer().removeMemoryRendering(AbstractAsyncTableRendering.this);
					return Status.CANCEL_STATUS;
				}
				
				final BigInteger finalMbBaseAddress = mbBaseAddress;
				final BigInteger initialSelectedAddress = getInitialSelectedAddress();
				
				if (monitor.isCanceled())
				{
					getMemoryRenderingContainer().removeMemoryRendering(AbstractAsyncTableRendering.this);
					return Status.CANCEL_STATUS;
				}
				
				createContentDescriptor(topVisibleAddress);
				
				// if it takes too long to get other information, and user has canceled
				// remove this rendering.
				if (monitor.isCanceled())
				{
					getMemoryRenderingContainer().removeMemoryRendering(AbstractAsyncTableRendering.this);
					return Status.CANCEL_STATUS;
				}
				
				// batch update on UI thread
				UIJob uiJob = new UIJob("Create Table Viewer UI Job"){ //$NON-NLS-1$
					public IStatus runInUIThread(IProgressMonitor progressMonitor) {
						
						if (fPageBook.isDisposed())
							return Status.OK_STATUS;
						
						fSashForm = new SashForm(parent, SWT.VERTICAL);
						fTableViewer = new AsyncTableRenderingViewer(AbstractAsyncTableRendering.this, fSashForm, SWT.VIRTUAL | SWT.SINGLE | SWT.H_SCROLL | SWT.V_SCROLL | SWT.HIDE_SELECTION | SWT.BORDER);

						GridData data = new GridData(GridData.FILL_BOTH);
						fTableViewer.getControl().setLayoutData(data);
						
						createGoToAddressComposite(fSashForm);
						hideGotoAddressComposite();
						
						IMemoryRenderingSite site = getMemoryRenderingContainer().getMemoryRenderingSite();
						IMemoryRenderingContainer container = getMemoryRenderingContainer();
						fPresentationContext = new MemoryViewPresentationContext(site, container, AbstractAsyncTableRendering.this);
						fTableViewer.setContext(fPresentationContext);
						
						// must call this after the context is created as the information is stored in the context
						getDynamicLoadFromPreference();
						getPageSizeFromPreference();
						
						int numberOfLines = getNumLinesToLoad();
						fContentDescriptor.setNumLines(numberOfLines);
						
						BigInteger baseAddress = finalMbBaseAddress;
						if (baseAddress == null)
							baseAddress = BigInteger.ZERO;
						
						
						if (!(getMemoryBlock() instanceof IMemoryBlockExtension) || !isDynamicLoad())
						{		
							// If not extended memory block, do not create any buffer
							// no scrolling
							fContentDescriptor.setPreBuffer(0);
							fContentDescriptor.setPostBuffer(0);
						} 
						
						setupInitialFormat();
						fTableViewer.setCellModifier(createCellModifier());
						fTableViewer.getTable().setHeaderVisible(true);
						fTableViewer.getTable().setLinesVisible(true);	
						fTableViewer.addPresentationErrorListener(fPresentationErrorListener);
						fTableViewer.setInput(getMemoryBlock());
						fTableViewer.resizeColumnsToPreferredSize();
						fTableViewer.setTopIndex(topVisibleAddress);
						
						fTableViewer.setSelection(initialSelectedAddress);

						// SET UP FONT		
						// set to a non-proportional font
						fTableViewer.getTable().setFont(JFaceResources.getFont(IInternalDebugUIConstants.FONT_NAME));
						
						if (!fError)
							showTable();
						
						fTableViewer.addVirtualContentListener(fViewerListener);
						
						// create context menu
						// create pop up menu for the rendering
						createActions();
						IMenuListener menuListener = new IMenuListener() {
							public void menuAboutToShow(IMenuManager mgr) {
								fillContextMenu(mgr);
							}
						};
						createPopupMenu(fTableViewer.getControl(), menuListener);
						createPopupMenu(fTableViewer.getCursor(), menuListener);
						
						fTableViewer.addSelectionChangedListener(fViewerSelectionChangedListener);
						fTableViewer.getTable().getVerticalBar().addSelectionListener(fScrollBarSelectionListener);
						
						createToolTip();
						
						// now the rendering is successfully created
						fIsCreated = true;
						
						return Status.OK_STATUS;
					}};
				uiJob.setSystem(true);
				uiJob.schedule();
					
				return Status.OK_STATUS;

			}};
			
		job.schedule();
	}
	
	/**
	 * Create popup menu for this rendering
	 * @param control - control to create the popup menu for
	 * @param menuListener - listener to notify when popup menu is about to show
	 */
	private void createPopupMenu(Control control, IMenuListener menuListener)
	{
		IMemoryRenderingContainer container = getMemoryRenderingContainer();
		if (fMenuMgr == null)
		{
			fMenuMgr = new MenuManager("#PopupMenu"); //$NON-NLS-1$
			fMenuMgr.setRemoveAllWhenShown(true);
			IMemoryRenderingSite site = container.getMemoryRenderingSite();
			String menuId = container.getId();
						
			ISelectionProvider selProvider = site.getSite().getSelectionProvider();
			
			addMenuListener(menuListener);

			site.getSite().registerContextMenu(menuId, fMenuMgr, selProvider);
		}
		
		addMenuListener(menuListener);
		
		Menu popupMenu = fMenuMgr.createContextMenu(control);
		control.setMenu(popupMenu);
	}


	private void addMenuListener(IMenuListener menuListener) {
		if (fMenuListeners == null)
			fMenuListeners = new ArrayList();
		
		if (!fMenuListeners.contains(menuListener))
		{
			fMenuMgr.addMenuListener(menuListener);
			fMenuListeners.add(menuListener);
		}
	}

	private BigInteger getInitialSelectedAddress() {
		// figure out selected address 
		BigInteger selectedAddress = (BigInteger) getSynchronizedProperty(AbstractAsyncTableRendering.PROPERTY_SELECTED_ADDRESS);
		if (selectedAddress == null)
		{
			if (getMemoryBlock() instanceof IMemoryBlockExtension) {
				try {
					selectedAddress = ((IMemoryBlockExtension) getMemoryBlock()).getBigBaseAddress();
				} catch (DebugException e) {
					selectedAddress = BigInteger.ZERO;
				}
				
				if (selectedAddress == null) {
					selectedAddress =BigInteger.ZERO;
				}

			} else {
				long address = getMemoryBlock().getStartAddress();
				selectedAddress = BigInteger.valueOf(address);
			}
		}
		return selectedAddress;
	}
	
	private void addListeners()
	{
		DebugUIPlugin.getDefault().getPreferenceStore().addPropertyChangeListener(this);
		addRenderingToSyncService();
		JFaceResources.getFontRegistry().addListener(this);
	}
	
	private void removeListeners()
	{
		DebugUIPlugin.getDefault().getPreferenceStore().removePropertyChangeListener(this);
		removeRenderingFromSyncService();
		JFaceResources.getFontRegistry().removeListener(this);
		
		Iterator iter = fMenuListeners.iterator();
		while (iter.hasNext())
		{
			fMenuMgr.removeMenuListener((IMenuListener)iter.next());
		}
		
		fMenuListeners.clear();
	}
	
	private void addRenderingToSyncService()
	{	
		IMemoryRenderingSynchronizationService syncService = getMemoryRenderingContainer().getMemoryRenderingSite().getSynchronizationService();
		
		if (syncService == null)
			return;
		
		syncService.addPropertyChangeListener(this, null);
	}
	
	private void removeRenderingFromSyncService()
	{
		IMemoryRenderingSynchronizationService syncService = getMemoryRenderingContainer().getMemoryRenderingSite().getSynchronizationService();
		
		if (syncService == null)
			return;
		
		syncService.removePropertyChangeListener(this);		
	}
	
	private void initAddressableSize()
	{
		// set up addressable size and figure out number of bytes required per line
		fAddressableSize = -1;
		try {
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
				fAddressableSize = ((IMemoryBlockExtension)getMemoryBlock()).getAddressableSize();
			else
				fAddressableSize = 1;
		} catch (DebugException e1) {
			DebugUIPlugin.log(e1);
			// log error and default to 1
			fAddressableSize = 1;
			return;
			
		}
		if (fAddressableSize < 1)
		{
			DebugUIPlugin.logErrorMessage("Invalid addressable size"); //$NON-NLS-1$
			fAddressableSize = 1;
		}
	}
	
	private BigInteger getInitialTopVisibleAddress() {
		BigInteger topVisibleAddress = (BigInteger) getSynchronizedProperty(AbstractAsyncTableRendering.PROPERTY_TOP_ADDRESS);
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

	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#getControl()
	 */
	public Control getControl() {
		return fPageBook.getParent();
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.util.IPropertyChangeListener#propertyChange(org.eclipse.jface.util.PropertyChangeEvent)
	 */
	public void propertyChange(PropertyChangeEvent event) {
		
		if (!fIsCreated)
			return;
		
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
		
		Object evtSrc = event.getSource();
		
		if (event.getProperty().equals(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE)) {
			// always update page size, only refresh if the table is visible
			getPageSizeFromPreference();
		}
		
		// do not handle event if the rendering is displaying an error
		// or if it's not visible
		if (isDisplayingError() || !isVisible())
		{
			handlePropertiesChangeWhenHidden(event);
			return;
		}
		
		
		if (event.getProperty().equals(IDebugUIConstants.PREF_PADDED_STR) || 
			event.getProperty().equals(IDebugUIConstants.PREF_CHANGED_DEBUG_ELEMENT_COLOR) ||
			event.getProperty().equals(IDebugUIConstants.PREF_MEMORY_HISTORY_KNOWN_COLOR) ||
			event.getProperty().equals(IDebugUIConstants.PREF_MEMORY_HISTORY_UNKNOWN_COLOR))
		{
			if (!fIsDisposed)
			{
				fTableViewer.refresh(false);
			}
			return;
		}
		
		if (event.getProperty().equals(IDebugPreferenceConstants.PREF_DYNAMIC_LOAD_MEM)) {
			handleDyanicLoadChanged();
			return;
		}
		
		if (event.getProperty().equals(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE)) {
			if (!isDynamicLoad())
			{
				int pageSize = DebugUIPlugin.getDefault().getPreferenceStore().getInt(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE);
				handlePageSizeChanged(pageSize);
			}
			return;
		}
		
		if (evtSrc == this)
			return;
		
		if (evtSrc instanceof IMemoryRendering)
		{
			IMemoryRendering rendering = (IMemoryRendering)evtSrc;
			IMemoryBlock memoryBlock = rendering.getMemoryBlock();
			
			// do not handle event from renderings displaying other memory blocks
			if (memoryBlock != getMemoryBlock())
				return;
		}
	
		String propertyName = event.getProperty();
		Object value = event.getNewValue();
		
		if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_SELECTED_ADDRESS) && value instanceof BigInteger)
		{
			selectedAddressChanged((BigInteger)value);
		}
		else if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_COL_SIZE) && value instanceof Integer)
		{
			columnSizeChanged(((Integer)value).intValue());
		}
		else if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_ROW_SIZE) && value instanceof Integer)
		{
			rowSizeChanged(((Integer)value).intValue());
		}
		else if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_TOP_ADDRESS) && value instanceof BigInteger)
		{
			topVisibleAddressChanged((BigInteger)value);
		}
		else if (propertyName.equals(IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS) && value instanceof BigInteger)
		{
			handlePageStartAddressChanged((BigInteger)value);
		}
	}

	/**
	 * 
	 */
	private void handlePageSizeChanged(int pageSize) {
		fPageSize = pageSize;
		// only refresh if in non-auto-load mode
		fContentDescriptor.setNumLines(pageSize);
		refresh();
	}	
	
	private void handlePropertiesChangeWhenHidden(PropertyChangeEvent event)
	{
		if (fPendingSyncProperties == null)
			return;
		
		String propertyName = event.getProperty();
		Object value = event.getNewValue();
		
		if (event.getSource() instanceof IMemoryRendering)
		{
			IMemoryRendering rendering = (IMemoryRendering)event.getSource();
			if (rendering == this || rendering.getMemoryBlock() != getMemoryBlock())
			{
				return;
			}
		}
		
		if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_COL_SIZE) && value instanceof Integer)
		{
			fPendingSyncProperties.setColumnSize(((Integer)value).intValue());
		}
		else if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_ROW_SIZE) && value instanceof Integer)
		{
			fPendingSyncProperties.setRowSize(((Integer)value).intValue());
		}
		
		else if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_SELECTED_ADDRESS) && value instanceof BigInteger)
		{
			fPendingSyncProperties.setSelectedAddress((BigInteger)value);
		}
		else if (propertyName.equals(AbstractAsyncTableRendering.PROPERTY_TOP_ADDRESS) && value instanceof BigInteger)
		{
			fPendingSyncProperties.setTopVisibleAddress((BigInteger)value);
		}
		else if (propertyName.equals(IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS) && value instanceof BigInteger)
		{
			fPendingSyncProperties.setPageStartAddress((BigInteger)value);
		}
		else if (event.getProperty().equals(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE)) {
			int pageSize = DebugUIPlugin.getDefault().getPreferenceStore().getInt(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE);
			fPendingSyncProperties.setPageSize(pageSize);
		}
	}
	
	private void topVisibleAddressChanged(final BigInteger address)
	{
		final Runnable runnable = new Runnable() {
			public void run() {
				if (fTableViewer.getTable().isDisposed())
					return;
				
				doTopVisibleAddressChanged(address);
			}};
		runOnUIThread(runnable);
	}
	
	/**
	 * @param address
	 */
	private void doTopVisibleAddressChanged(final BigInteger address) {
		if (fIsDisposed)
			return;
		
		if (!isDynamicLoad())
		{
			fTableViewer.setTopIndex(address);
			fTableViewer.topIndexChanged();
			return;
		}
		
		if (!isAtTopBuffer(address) && !isAtBottomBuffer(address))
		{
			fTableViewer.setTopIndex(address);
			fTableViewer.topIndexChanged();
		}
		else
		{
			reloadTable(address);
		}
	}
	
	private boolean isAtBottomBuffer(BigInteger address)
	{
		int idx = fTableViewer.indexOf(address);
		if (idx < 0)
			return true;
		
		int bottomIdx = idx + getNumberOfVisibleLines();
		int elementsCnt = fTableViewer.getVirtualContentModel().getElements().length;
		int numLinesLeft = elementsCnt - bottomIdx;
		
		if (numLinesLeft <= 3)
			return true;
		
		return false;
	}
	
	private boolean isAtTopBuffer(BigInteger address)
	{
		int topIdx = fTableViewer.indexOf(address);
		if (topIdx <= 3)
			return true;
		
		return false;
	}
	
	private void runOnUIThread(final Runnable runnable)
	{
		if (Display.getCurrent() != null)	
		{
			runnable.run();
		}
		else
		{
			UIJob job = new UIJob("Async Table Rendering UI Job"){ //$NON-NLS-1$
	
				public IStatus runInUIThread(IProgressMonitor monitor) {
					runnable.run();
					return Status.OK_STATUS;
				}};
			job.setSystem(true);
			job.schedule();
		}
	}
	
	private void selectedAddressChanged(final BigInteger address)
	{
		Runnable runnable = new Runnable() {

			public void run() {			
				
				if (fTableViewer.getTable().isDisposed())
					return;
				
				// call this to make the table viewer to reload when needed
				int i = fTableViewer.indexOf(address);
				if (i < 0)
				{
					topVisibleAddressChanged(address);
				}
				fTableViewer.setSelection(address);
			}
		};
		
		runOnUIThread(runnable);
	}
	
	private void setFont(Font font)
	{	
		// set font
		fTableViewer.getTable().setFont(font);
		fTableViewer.getCursor().setFont(font);		
	}
	
	private int getDefaultColumnSize() {
		
		// default to global preference store
		IPreferenceStore prefStore = DebugUITools.getPreferenceStore();
		int columnSize = prefStore.getInt(IDebugPreferenceConstants.PREF_COLUMN_SIZE);
		// actual column size is number of addressable units * size of the addressable unit
		columnSize = columnSize * getAddressableSize();
		
		// check synchronized col size
		Integer colSize = (Integer)getSynchronizedProperty(AbstractAsyncTableRendering.PROPERTY_COL_SIZE);
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
		Integer size = (Integer)getSynchronizedProperty(AbstractAsyncTableRendering.PROPERTY_ROW_SIZE);
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
	
	
	private String getRowPrefId(String modelId) {
		String rowPrefId = IDebugPreferenceConstants.PREF_ROW_SIZE + ":" + modelId; //$NON-NLS-1$
		return rowPrefId;
	}

	private String getColumnPrefId(String modelId) {
		String colPrefId = IDebugPreferenceConstants.PREF_COLUMN_SIZE + ":" + modelId; //$NON-NLS-1$
		return colPrefId;
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
		formatViewer();

		updateSyncRowSize();
		updateSyncColSize();
		
		return true;
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
				if (items[i].getData() != null)
					minHeight = Math.min(items[i].getBounds(0).height, minHeight);
			}
			
			return minHeight;
				
		}
		return table.getItemHeight();
	}
	
	private BigInteger getMemoryBlockBaseAddress() throws DebugException
	{
		if (getMemoryBlock() instanceof IMemoryBlockExtension)
			return ((IMemoryBlockExtension)getMemoryBlock()).getBigBaseAddress();
		else
			return BigInteger.valueOf(getMemoryBlock().getStartAddress());
	}
	
	/**
	 * Displays the given message on the error page
	 * @param message - the message to display
	 */
	protected void showMessage(final String message)
	{
		fSwitchPageJob.setShowMessagePage(true);
		fSwitchPageJob.setMessage(message);
		fSwitchPageJob.schedule();
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
	 * Returns whether the error page is displayed.
	 * 
	 * @return whether the error page is displayed
	 */
	public boolean isDisplayingError()
	{
		return fShowMessage;
	}
	
	/**
	 * Displays the content of the table viewer.
	 */
	public void showTable()
	{
		fSwitchPageJob.setShowMessagePage(false);
		fSwitchPageJob.schedule();
	}
	
	private BigInteger getTopVisibleAddress() {
		
		if (fTableViewer == null)
			return BigInteger.valueOf(0);

		Table table = fTableViewer.getTable();
		int topIndex = table.getTopIndex();

		if (topIndex < 0) { return null; }

		if (table.getItemCount() > topIndex) 
		{
			MemorySegment topItem = (MemorySegment)table.getItem(topIndex).getData();
			if (topItem != null)
			{
				return topItem.getAddress();
			}
		}
		return null;
	}
	
	private  synchronized void  reloadTable(final BigInteger topAddress) {
		
		if (AsyncVirtualContentTableViewer.DEBUG_DYNAMIC_LOADING)
			System.out.println(this + " reload at: " + topAddress.toString(16)); //$NON-NLS-1$
		
		fContentDescriptor.setLoadAddress(topAddress);
		fContentDescriptor.setNumLines(getNumLinesToLoad());
		fTableViewer.setTopIndex(topAddress);
		fTableViewer.refresh();

	}
	
	private boolean isAtTopLimit()
	{	
		BigInteger startAddress = fContentDescriptor.getStartAddress();
		startAddress = MemoryViewUtil.alignToBoundary(startAddress, getAddressableUnitPerLine());
		AbstractVirtualContentTableModel model = fTableViewer.getVirtualContentModel();
		
		if (model != null)
		{
			Object key = model.getKey(0);
			if (key instanceof BigInteger)
			{
				BigInteger startBufferAddress = (BigInteger)key;
				startBufferAddress = MemoryViewUtil.alignToBoundary(startBufferAddress, getAddressableUnitPerLine());
				
				if (startAddress.compareTo(startBufferAddress) == 0)
					return true;
			}
		}
		return false;
	}
	
	private boolean isAtBottomLimit()
	{
		BigInteger endAddress = fContentDescriptor.getEndAddress();
		endAddress = MemoryViewUtil.alignToBoundary(endAddress, getAddressableUnitPerLine());
		
		AbstractVirtualContentTableModel model = fTableViewer.getVirtualContentModel();
		if (model != null)
		{
			int numElements = model.getElements().length;
			Object key = model.getKey(numElements-1);
			if (key instanceof BigInteger)
			{
				BigInteger endBufferAddress = (BigInteger)key;
				endBufferAddress = MemoryViewUtil.alignToBoundary(endBufferAddress, getAddressableUnitPerLine());
				
				if (endAddress.compareTo(endBufferAddress) == 0)
					return true;
			}
		}
		
		return false;		
	}
	
	private void formatViewer() {
		
		fTableViewer.disposeColumns();
		fTableViewer.disposeCellEditors();
		doFormatTable();
		fTableViewer.setColumnHeaders(getColumnProperties());
		fTableViewer.showColumnHeader(true);
		fTableViewer.setCellEditors(createCellEditors(fTableViewer.getTable()));
		
		fTableViewer.formatViewer();
		
		// This resize needs to happen after the viewer has finished
		// getting the labels.
		// This fix is a hack to delay the resize until the viewer has a chance to get
		// the setData event from the UI thread.  Otherwise, the columns will be
		// squeezed together.
		UIJob job = new UIJob("resize to fit"){ //$NON-NLS-1$
			public IStatus runInUIThread(IProgressMonitor monitor) {
				resizeColumnsToPreferredSize();
				return Status.OK_STATUS;
			}};
		
		job.setSystem(true);
		job.schedule();
	}

	private void doFormatTable() {
		int bytesPerLine = getBytesPerLine();
		int columnSize = getBytesPerColumn();
		int numColumns = bytesPerLine/columnSize;
		
		Table table = fTableViewer.getTable();
		TableColumn column0 = new TableColumn(table,SWT.LEFT,0);
		column0.setText(DebugUIMessages.AbstractTableRendering_2); 
		
		// create new byte columns
		TableColumn [] byteColumns = new TableColumn[numColumns];		
		for (int i=0;i<byteColumns.length; i++)
		{
			TableColumn column = new TableColumn(table, SWT.LEFT, i+1);
			byteColumns[i] = column;
		}
		
		//Empty column for cursor navigation
		TableColumn emptyCol = new TableColumn(table,SWT.LEFT,byteColumns.length+1);
		emptyCol.setText(" "); //$NON-NLS-1$
		emptyCol.setWidth(1);
		emptyCol.setResizable(false);
	    table.setHeaderVisible(true);
	    
	    // allow clients to override column labels
	   setColumnHeadings();
	    
	}
	
	private String[] getColumnProperties()
	{
		int numColumns = getAddressableUnitPerLine()/getAddressableUnitPerColumn();
		// +2 to include properties for address and navigation column
		String[] columnProperties = new String[numColumns+2];
		columnProperties[0] = TableRenderingLine.P_ADDRESS;
		
		int addressableUnit = getAddressableUnitPerColumn();

		// use column beginning offset to the row address as properties
		for (int i=1; i<columnProperties.length-1; i++)
		{
			// column properties are stored as number of addressable units from the
			// the line address
			columnProperties[i] = Integer.toHexString((i-1)*addressableUnit);
		}
		
		// Empty column for cursor navigation
		columnProperties[columnProperties.length-1] = " "; //$NON-NLS-1$
		return columnProperties;
	}
	
   private CellEditor[] createCellEditors(Table table) {
        CellEditor[] editors = new CellEditor[table.getColumnCount()];
        for (int i=0; i<editors.length; i++)
        {
        	editors[i] = new TextCellEditor(table);
        }
        return editors;
    }
   
   	private ICellModifier createCellModifier() {
       return new AsyncTableRenderingCellModifier(this);
   	}

	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.AbstractMemoryRendering#dispose()
	 */
	public void dispose() {
		
		if (fIsDisposed)
			return;
		
		fIsDisposed = true;
		
		removeListeners();
		
		if (fMenuMgr != null)
		{
			fMenuMgr.removeAll();
			fMenuMgr.dispose();
			fMenuMgr = null;
		}
		
		if (fTableViewer != null)
		{
			if (fViewerListener != null)
				fTableViewer.removeVirtualContentListener(fViewerListener);
			
			if (fPresentationErrorListener != null)
				fTableViewer.removePresentationErrorListener(fPresentationErrorListener);
			
			fTableViewer.removeSelectionChangedListener(fViewerSelectionChangedListener);
			fTableViewer.getTable().getVerticalBar().removeSelectionListener(fScrollBarSelectionListener);
			
			fTableViewer.dispose();
		}
		
		fIsDisposed = true;
		
		super.dispose();
	}
	
	/**
	 * Updates the label of this rendering, optionally displaying the
	 * base address of this rendering's memory block.
	 * 
	 * @param showAddress whether to display the base address of this
	 *  rendering's memory block in this rendering's label
	 */
	protected void updateRenderingLabel(final boolean showAddress)
	{
		Job job = new Job("Update Rendering Label"){ //$NON-NLS-1$
			protected IStatus run(IProgressMonitor monitor) {
				if (fIsDisposed)
					return Status.OK_STATUS;
				fLabel = buildLabel(showAddress);
				firePropertyChangedEvent(new PropertyChangeEvent(AbstractAsyncTableRendering.this, IBasicPropertyConstants.P_TEXT, null, fLabel));
				return Status.OK_STATUS;
			}};
		job.setSystem(true);
		job.setRule(serialByRenderingRule);
		job.schedule();
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
	
	/* Returns the label of this rendering.
	 * 
	 * @return label of this rendering
	 */
	public String getLabel() {
		
		if (fLabel == null)
		{
			fLabel = DebugUIMessages.AbstractAsyncTableRendering_1;
			updateRenderingLabel(isVisible());
		}
		
		return fLabel;
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.core.runtime.PlatformObject#getAdapter(java.lang.Class)
	 */
	public Object getAdapter(Class adapter) {
		
		if (adapter == IColorProvider.class)
			return getColorProviderAdapter();
		
		if (adapter == ILabelProvider.class)
			return getLabelProviderAdapter();
		
		if (adapter == IFontProvider.class)
			return getFontProviderAdapter();
		
		if (adapter == IModelChangedListener.class)
		{
			return fModelChangedListener;
		}
		
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
						return AbstractAsyncTableRendering.this.getLabel();
					}
	
					public Object getParent(Object o) {
						return null;
					}
				};
			}
			return fWorkbenchAdapter;
		}
		
		if (adapter == TableRenderingContentDescriptor.class)
			return getContentDescriptor();
		
		return super.getAdapter(adapter);
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
	
	/**
	 * Create actions for this rendering
	 */
	protected void createActions() {
		
		fCopyToClipboardAction = new AsyncCopyTableRenderingAction(this, fTableViewer);
		fGoToAddressAction = new RenderingGoToAddressAction(this);
		fResetMemoryBlockAction = new ResetToBaseAddressAction(this);
		
		fPrintViewTabAction = new AsyncPrintTableRenderingAction(this, fTableViewer);
		
		fFormatRenderingAction = new FormatTableRenderingAction(this);		
		fReformatAction = new ReformatAction(this);
		fToggleAddressColumnAction = new ToggleAddressColumnAction();
		
		IMemoryRenderingSite site = getMemoryRenderingContainer().getMemoryRenderingSite();
		if (site.getSite().getSelectionProvider() != null)
		{
			fPropertiesDialogAction = new PropertyDialogAction(site.getSite(),site.getSite().getSelectionProvider()); 
		}
		
		fNextAction = new NextPageAction();
		fPrevAction = new PrevPageAction();
	}
	
	/**
	 * Returns the currently selected address in this rendering.
	 * 
	 * @return the currently selected address in this rendering
	 */
	public BigInteger getSelectedAddress() {
		Object key = fTableViewer.getSelectionKey();
		
		if (key != null && key instanceof BigInteger)
			return (BigInteger)key;
		
		return null;
	}

	/**
	 * Returns the currently selected content in this rendering as MemoryByte.
	 * 
	 * @return the currently selected content in array of MemoryByte.  
	 * Returns an empty array if the selected address is out of buffered range.
	 */
	public MemoryByte[] getSelectedAsBytes()
	{
		if (getSelectedAddress() == null)
			return new MemoryByte[0];
		
		Object key = fTableViewer.getSelectionKey();
		AbstractVirtualContentTableModel model = fTableViewer.getVirtualContentModel();
		
		if (model != null)
		{
			model = (AbstractVirtualContentTableModel)fTableViewer.getModel();
			int row = model.indexOfKey(key);
			Object element = model.getElement(row);
			int col = model.columnOf(element, key);
		
			// check precondition
			if (col <= 0 || col > getBytesPerLine()/getBytesPerColumn())
			{
				return new MemoryByte[0];
			}
			
			if (!(element instanceof MemorySegment))
				return new MemoryByte[0];
			
			MemorySegment line = (MemorySegment)element;
			int offset = (col-1)*(getAddressableUnitPerColumn()*getAddressableSize());
			
			// make a copy of the bytes to ensure that data cannot be changed
			// by caller
			MemoryByte[] bytes = line.getBytes(offset, getAddressableUnitPerColumn()*getAddressableSize());
			MemoryByte[] retBytes = new MemoryByte[bytes.length];
			
			System.arraycopy(bytes, 0, retBytes, 0, bytes.length);
			return retBytes;
		}
		return new MemoryByte[0];
	}

	/**
	 * Returns the currently selected content in this rendering as a String.
	 * 
	 * @return the currently selected content in this rendering
	 */
	public String getSelectedAsString() {

		if (getSelectedAddress() == null)
			return ""; //$NON-NLS-1$
		
		MemoryByte[] bytes = getSelectedAsBytes();
		if (bytes.length > 0)
		{
			return getString(this.getRenderingId(), getSelectedAddress(), bytes);
		}
		else
			return ""; //$NON-NLS-1$
		
	}

	/**
	 * Moves the cursor to the specified address.
	 * Will load more memory if the address is not currently visible.
	 * 
	 * @param address address to position cursor at
	 * @throws DebugException if an exception occurs
	 */
	public void goToAddress(BigInteger address) throws DebugException {
		
		if (fTableViewer.getVirtualContentModel() == null)
			return;
		
		int i = fTableViewer.getVirtualContentModel().indexOfKey(address);

		if (i >= 0)
		{	
			// address is within range, set cursor and reveal
			fTableViewer.setSelection(address);
			updateSyncTopAddress(getTopVisibleAddress());
			updateSyncSelectedAddress(address);
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

			BigInteger startAdd = fContentDescriptor.getStartAddress();
			BigInteger endAdd = fContentDescriptor.getEndAddress();
			
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
	
			// load at the address
			fTableViewer.setSelection(address);
			reloadTable(address);
	
			updateSyncSelectedAddress(address);

			if (!isDynamicLoad())
			{						
				updateSyncPageStartAddress(address);
			}
			
			updateSyncTopAddress(address);
		}
	}			
	
	/**
	 * Refresh the table viewer with the current top visible address.
	 * Update labels in the memory rendering.
	 */
	public void refresh() {
		fTableViewer.refresh();
	}

	
	/**
	 * Resize column to the preferred size.
	 */
	public void resizeColumnsToPreferredSize() {
		fTableViewer.resizeColumnsToPreferredSize();
		if (!fIsShowAddressColumn)
		{
			final TableColumn column = fTableViewer.getTable().getColumn(0);
			column.addControlListener(new ControlListener() {

				public void controlMoved(ControlEvent e) {
				}

				public void controlResized(ControlEvent e) {
					column.removeControlListener(this);
					column.setWidth(0);
				}});
		}
	}

	/**
	 * Updates labels of this rendering.
	 */
	public void updateLabels()
	{
		UIJob job = new UIJob("updateLabels"){ //$NON-NLS-1$

			public IStatus runInUIThread(IProgressMonitor monitor) {
							
				// do not handle if the rendering is already disposed
				if (fPageBook.isDisposed())
					return Status.OK_STATUS;
				
				// update tab labels
				updateRenderingLabel(true);
				
				if (fTableViewer != null)
				{
					// update column labels
					setColumnHeadings();
					
					// rebuild cache and force labels to be refreshed
					fTableViewer.formatViewer();
				}
				return Status.OK_STATUS;
			}};
		job.setSystem(true);
		job.schedule();
	}
	
	/**
	 * Fills the context menu for this rendering
	 * 
	 * @param menu menu to fill
	 */
	protected void fillContextMenu(IMenuManager menu) {	
		
		menu.add(new Separator(EMPTY_MEMORY_GROUP));
		menu.add(new Separator());
		menu.add(fResetMemoryBlockAction);
		menu.add(fGoToAddressAction);
		menu.add(new Separator(EMPTY_NAVIGATION_GROUP));
		
		menu.add(new Separator());
		menu.add(fFormatRenderingAction);

		if (!isDynamicLoad() && getMemoryBlock() instanceof IMemoryBlockExtension)
		{		
			menu.add(new Separator());
			menu.add(fPrevAction);
			menu.add(fNextAction);
			menu.add(new Separator(EMPTY_NON_AUTO_LOAD_GROUP));
		}
		
		menu.add(new Separator());
		menu.add(fReformatAction);
		menu.add(fToggleAddressColumnAction);
		menu.add(new Separator());
		menu.add(fCopyToClipboardAction);
		menu.add(fPrintViewTabAction);
		if (fPropertiesDialogAction != null)
		{
			menu.add(new Separator());
			menu.add(fPropertiesDialogAction);
			menu.add(new Separator(EMPTY_PROPERTY_GROUP));
		}
		
		menu.add(new Separator(IWorkbenchActionConstants.MB_ADDITIONS));
	}
	
	private int getPageSizeInUnits()
	{
		return fPageSize * getAddressableUnitPerLine();
	}
	
	private void getPageSizeFromPreference()
	{
		fPageSize = DebugUIPlugin.getDefault().getPreferenceStore().getInt(IDebugPreferenceConstants.PREF_TABLE_RENDERING_PAGE_SIZE);
	}
	
	private void updateDynamicLoadProperty() {
		
		boolean value = DebugUIPlugin
				.getDefault()
				.getPreferenceStore()
				.getBoolean(IDebugPreferenceConstants.PREF_DYNAMIC_LOAD_MEM);
		
		if (value != isDynamicLoad())
		{
			setDynamicLoad(value);
		
			if (!fIsDisposed) {
				if (isDynamicLoad()) {
					fContentDescriptor.setPostBuffer(20);
					fContentDescriptor.setPreBuffer(20);
					fContentDescriptor.setNumLines(getNumberOfVisibleLines());
	
				} else {
					fContentDescriptor.setPostBuffer(0);
					fContentDescriptor.setPreBuffer(0);
					fContentDescriptor.setNumLines(fPageSize);
				}	
			}
		}
	}
	
	private void getDynamicLoadFromPreference()
	{
		setDynamicLoad(DebugUIPlugin.getDefault().getPreferenceStore().getBoolean(IDebugPreferenceConstants.PREF_DYNAMIC_LOAD_MEM));
	}
	
	private boolean isDynamicLoad()
	{
		return fContentDescriptor.isDynamicLoad();
	}
	
	private int getPageSize()
	{
		return fPageSize;
	}
	
	private int getNumLinesToLoad() {
		int numberOfLines = -1;
		
		if (isDynamicLoad())
			numberOfLines = getNumberOfVisibleLines();
		else
			numberOfLines = getPageSize();
		
		return numberOfLines;
	}
	
	private void setDynamicLoad(boolean load)
	{
		fContentDescriptor.setDynamicLoad(load);
	}
	
	private void handlePageStartAddressChanged(BigInteger address)
	{
		// do not handle if in dynamic mode
		if (isDynamicLoad())
			return;
		
		if (!(getMemoryBlock() instanceof IMemoryBlockExtension))
			return;
		
		// do not handle event if the base address of the memory
		// block has changed, wait for debug event to update to
		// new location
		if (isMemoryBlockBaseAddressChanged())
			return;

		if(fTableViewer.getKey(0).equals(address))
			return;
	
		BigInteger start = fContentDescriptor.getStartAddress();
		BigInteger end = fContentDescriptor.getEndAddress();
		
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
		
		fContentDescriptor.setLoadAddress(address);
		final BigInteger finaladdress = address;
		Runnable runnable = new Runnable() {
			public void run() {
				if (fTableViewer.getTable().isDisposed())
					return;
				
				fTableViewer.setTopIndex(finaladdress);
				refresh();
			}};
		
		runOnUIThread(runnable);

		updateSyncPageStartAddress(address);
		updateSyncTopAddress(address);
	}
	private void handleDyanicLoadChanged() {
		
		// if currently in dynamic load mode, update page
		// start address
		BigInteger pageStart = getTopVisibleAddress();
		updateSyncPageStartAddress(pageStart);
		
		updateDynamicLoadProperty();
		if (isDynamicLoad())
		{
			refresh();
			fTableViewer.setTopIndex(pageStart);
		}
		else
		{
			handlePageStartAddressChanged(pageStart);
		}
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#becomesHidden()
	 */
	public void becomesHidden() {		
		// creates new object for storing potential changes in sync properties
		fPendingSyncProperties = new PendingPropertyChanges();		
		super.becomesHidden();
		
		if (getMemoryBlock() instanceof IMemoryBlockExtension)
			updateRenderingLabel(false);	
	}

	/* (non-Javadoc)
	 * @see org.eclipse.debug.ui.memory.IMemoryRendering#becomesVisible()
	 */
	public void becomesVisible() {
		
		if (!fIsCreated)
		{
			// label can still be constructed even though the rendering has not finished being
			// initialized
			updateRenderingLabel(true);
			super.becomesVisible();
			return;
		}
		
		// do not do anything if already visible
		if (isVisible() == true)
		{
			// super should always be called
			super.becomesVisible();
			return;
		}
		
		super.becomesVisible();
		
		if (fPendingSyncProperties != null)
		{
			// deal with format
			boolean format = false;
			int rowSize = getBytesPerLine();
			if (fPendingSyncProperties.getRowSize() > 0)
			{
				format = true;
				rowSize = fPendingSyncProperties.getRowSize();
			}
			
			int colSize = getBytesPerColumn();
			if (fPendingSyncProperties.getColumnSize() > 0)
			{
				format = true;
				colSize = fPendingSyncProperties.getColumnSize();
			}
			
			if (format)
				format(rowSize, colSize);
			
			BigInteger selectedAddress = fPendingSyncProperties.getSelectedAddress();
			if (selectedAddress != null)
				fTableViewer.setSelection(selectedAddress);
			
			updateDynamicLoadProperty();
			
			if (isDynamicLoad())
			{
				BigInteger topVisibleAddress = fPendingSyncProperties.getTopVisibleAddress();
				if (topVisibleAddress != null)
				{
					fContentDescriptor.setLoadAddress(topVisibleAddress);
					fTableViewer.setTopIndex(topVisibleAddress);
				}
			}
			else if (!(getMemoryBlock() instanceof IMemoryBlockExtension))
			{
				BigInteger topVisibleAddress = fPendingSyncProperties.getTopVisibleAddress();
				if (topVisibleAddress != null)
					fTableViewer.setTopIndex(topVisibleAddress);
			}
			else
			{
				if (fPendingSyncProperties.getPageSize() > 0)
				{
					fPageSize = fPendingSyncProperties.getPageSize();
					fContentDescriptor.setNumLines(fPageSize);
				}
				
				BigInteger pageStartAddress = fPendingSyncProperties.getPageStartAddress();
				if (pageStartAddress != null)
					fContentDescriptor.setLoadAddress(pageStartAddress);
				
				fTableViewer.setTopIndex(pageStartAddress);
			}
			
			showTable();
			refresh();
		}

		updateRenderingLabel(true);
		
		Job job = new Job("becomesVisible") //$NON-NLS-1$
		{
			protected IStatus run(IProgressMonitor monitor) {
				if (fIsDisposed)
					return Status.OK_STATUS;
				try {
					fContentDescriptor.updateContentBaseAddress();
				} catch (DebugException e) {
					showMessage(e.getMessage());
				}
				return Status.OK_STATUS;
			}
		};
		job.setSystem(true);
		job.schedule();
		
		// discard these properties
		fPendingSyncProperties = null;
	}
	
	/**
	 * Handle column size changed event from synchronizer
	 * @param newColumnSize
	 */
	private void columnSizeChanged(final int newColumnSize) {
		// ignore event if rendering is not visible
		if (!isVisible())
			return;

		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				int rowSize = getBytesPerLine();
				if (rowSize < newColumnSize)
					rowSize = newColumnSize;
					
				format(rowSize, newColumnSize);
			}
		});
	}
	
	/**
	 * @param newRowSize - new row size in number of bytes
	 */
	private void rowSizeChanged(final int newRowSize)
	{
		// ignore event if rendering is not visible
		if (!isVisible())
			return;
		
		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				int colSize = getBytesPerColumn();
				if (newRowSize < colSize)
					colSize = newRowSize;
				
				format(newRowSize, colSize);
			}
		});		
	}
	
	/**
	 * update selected address in synchronizer if update is true.
	 */
	private void updateSyncSelectedAddress(BigInteger address) {
		
		if (!fIsCreated)
			return;
		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractAsyncTableRendering.PROPERTY_SELECTED_ADDRESS, null, address);
		firePropertyChangedEvent(event);
	}
	
	/**
	 * update column size in synchronizer
	 */
	private void updateSyncColSize() {
		
		if (!fIsCreated)
			return;
		
		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractAsyncTableRendering.PROPERTY_COL_SIZE, null, new Integer(fColumnSize));
		firePropertyChangedEvent(event);
	}
	
	/**
	 * update column size in synchronizer
	 */
	private void updateSyncRowSize() {
		
		if (!fIsCreated)
			return;
		
		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractAsyncTableRendering.PROPERTY_ROW_SIZE, null, new Integer(fBytePerLine));
		firePropertyChangedEvent(event);
	}
	
	/**
	 * update top visible address in synchronizer
	 */
	private void updateSyncTopAddress(BigInteger address) {
		
		if (!fIsCreated)
			return;

		PropertyChangeEvent event = new PropertyChangeEvent(this, AbstractAsyncTableRendering.PROPERTY_TOP_ADDRESS, null, address);
		firePropertyChangedEvent(event);
	}
	
	private void updateSyncPageStartAddress(BigInteger address) {
	
		if (!fIsCreated)
			return;
		
		if (isMemoryBlockBaseAddressChanged())
			return;
		
		PropertyChangeEvent event = new PropertyChangeEvent(this, IInternalDebugUIConstants.PROPERTY_PAGE_START_ADDRESS, null, address);
		firePropertyChangedEvent(event);
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
		fTableViewer.getCursor().addMouseTrackListener(listener);
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
			TableItem item = items[i];
			if (item.getData() != null)
			{
				Point start = new Point(item.getBounds(0).x, item.getBounds(0).y);
				start = fTableViewer.getTable().toDisplay(start);
				Point end = new Point(start.x + item.getBounds(0).width, start.y + item.getBounds(0).height);
				
				if (start.y < point.y && point.y < end.y)
					return item;
			}
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
		
		TableItem item = null;
		for (int i=0; i<fTableViewer.getTable().getItemCount(); i++)
		{
			item = fTableViewer.getTable().getItem(i);
			if (item.getData() != null)
				break;
		}
		
		if (item != null)
		{
			for (int i=0; i<colCnt; i++)
			{
				Point start = new Point(item.getBounds(i).x, item.getBounds(i).y);
				start = fTableViewer.getTable().toDisplay(start);
				Point end = new Point(start.x + item.getBounds(i).width, start.y + item.getBounds(i).height);
				
				if (start.x < point.x && end.x > point.x)
					return i;
			}
		}
		return -1;
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
			Object address = fTableViewer.getKey(fTableViewer.getTable().indexOf(item), col);
			if (address != null  && address instanceof BigInteger) {
				Object data = item.getData();
				if (data instanceof MemorySegment) {
					MemorySegment line = (MemorySegment) data;

					if (col > 0) {
						int start = (col - 1) * getBytesPerColumn();
						int end = start + getBytesPerColumn();
						MemoryByte[] bytes = line.getBytes(start, end);

						String str = getToolTipText((BigInteger)address, bytes);

						if (str != null)
							((Label) toolTipControl).setText(str);
					} else {
						String str = getToolTipText((BigInteger)address,
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
	
	private void setColumnHeadings()
	{
		String[] columnLabels = new String[0];
		
		IMemoryBlockTablePresentation presentation = getTablePresentationAdapter();
		if (presentation != null)
		{
			columnLabels = presentation.getColumnLabels(getMemoryBlock(), getBytesPerLine(), getBytesPerLine()/getBytesPerColumn());
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
				int addressableUnit = getAddressableUnitPerColumn();
				if (addressableUnit >= 4)
				{
					columns[i].setText(Integer.toHexString(j*addressableUnit).toUpperCase() + 
							" - " + Integer.toHexString(j*addressableUnit+addressableUnit-1).toUpperCase()); //$NON-NLS-1$
				}
				else
				{
					columns[i].setText(Integer.toHexString(j*addressableUnit).toUpperCase());
				}
				j++;
			}
		}
	}
	
	/**
	 * 
	 * Return this rendering's viewer
	 * @return this rendering's viewer
	 */
	public StructuredViewer getViewer()
	{
		return fTableViewer;
	}
	
	private boolean isMemoryBlockBaseAddressChanged()
	{
		try {
			BigInteger address = getMemoryBlockBaseAddress();
			BigInteger oldBaseAddress = fContentDescriptor.getContentBaseAddress();
			if (!oldBaseAddress.equals(address))
				return true;
		} catch (DebugException e) {
			// fail silently
		}
		return false;
	}
	
	/**
	 * @param topVisibleAddress
	 */
	private void createContentDescriptor(final BigInteger topVisibleAddress) {
		fContentDescriptor = new TableRenderingContentDescriptor(AbstractAsyncTableRendering.this);
		fContentDescriptor.setPostBuffer(20);
		fContentDescriptor.setPreBuffer(20);
		fContentDescriptor.setLoadAddress(topVisibleAddress);
		try {
			fContentDescriptor.updateContentBaseAddress();
			
		} catch (DebugException e) {
			fError = true;
			showMessage(e.getMessage());
		}
		
		fContentDescriptor.setAddressableSize(getAddressableSize());
			
		try {
			int addressSize = 4;
			if (getMemoryBlock() instanceof IMemoryBlockExtension)
			{
				IMemoryBlockExtension extMb = (IMemoryBlockExtension)getMemoryBlock();
				addressSize = extMb.getAddressSize();
				
				if (addressSize <= 0)
				{
					DebugUIPlugin.logErrorMessage("Invalid address Size: " + addressSize); //$NON-NLS-1$
					addressSize = 4;
				}
				fContentDescriptor.setAddressSize(addressSize);
			}
			fContentDescriptor.setAddressSize(addressSize);
		} catch (DebugException e) {
			fError = true;
			showMessage(e.getMessage());
		} finally {
			if (fContentDescriptor.getAddressSize() <= 0)
				fContentDescriptor.setAddressSize(4);
		}
		
	}
	
	private TableRenderingContentDescriptor getContentDescriptor()
	{
		return fContentDescriptor;
	}
	
	private void createGoToAddressComposite(Composite parent)
	{
		fGoToAddressComposite = new GoToAddressComposite();
		fGoToAddressComposite.createControl(parent);
		Button button = fGoToAddressComposite.getButton(IDialogConstants.OK_ID);
		if (button != null)
		{
			button.addSelectionListener(new SelectionAdapter() {

				public void widgetSelected(SelectionEvent e) {
					doGoToAddress();
				}
			});
			
			button = fGoToAddressComposite.getButton(IDialogConstants.CANCEL_ID);
			if (button != null)
			{
				button.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						hideGotoAddressComposite();
					}});
			}
		}
		
		fGoToAddressComposite.getExpressionWidget().addSelectionListener(new SelectionAdapter() {
			public void widgetDefaultSelected(SelectionEvent e) {
				doGoToAddress();
			}});
		
		fGoToAddressComposite.getExpressionWidget().addKeyListener(new KeyAdapter() {

			public void keyPressed(KeyEvent e) {
				if (e.keyCode == SWT.ESC)
					hideGotoAddressComposite();
				super.keyPressed(e);
			}});
	}
	
	private void showGoToAddressComposite() {
		
		String selectedStr = getSelectedAsString();
		Text text = fGoToAddressComposite.getExpressionWidget();
		text.setText(selectedStr);
		text.setSelection(0, text.getCharCount());
	
		double height = fGoToAddressComposite.getHeight();
		double canvasHeight = fSashForm.getParent().getClientArea().height;
		double tableHeight = canvasHeight - height;
		
		double tableWeight = (tableHeight/canvasHeight) * 100;
		double textWeight = (height / canvasHeight) * 100;
		fSashForm.setWeights(new int[]{(int)tableWeight, (int)textWeight});
		fSashForm.setMaximizedControl(null);
		
		fGoToAddressComposite.getExpressionWidget().setFocus();
	}
	
	private void hideGotoAddressComposite()
	{
		fSashForm.setMaximizedControl(fTableViewer.getControl());
		if (isActivated())
			fTableViewer.getControl().setFocus();
	}
	
	/**
	 * 
	 */
	private void doGoToAddress() {
		try {
			BigInteger address = fGoToAddressComposite.getGoToAddress(fContentDescriptor.getContentBaseAddress(), getSelectedAddress());
			fGoToAddressAction.doGoToAddress(address.toString(16));
			hideGotoAddressComposite();
		} catch (DebugException e1) {
			MemoryViewUtil.openError(DebugUIMessages.GoToAddressAction_Go_to_address_failed, 
					DebugUIMessages.GoToAddressAction_Go_to_address_failed, e1);
		} catch (NumberFormatException e1)
		{
			MemoryViewUtil.openError(DebugUIMessages.GoToAddressAction_Go_to_address_failed, 
				DebugUIMessages.GoToAddressAction_Address_is_invalid, e1);
		}
	}
	
	public void activated() {
		super.activated();
		
		fActivated = true;
		IWorkbench workbench = PlatformUI.getWorkbench();
		ICommandService commandSupport = (ICommandService)workbench.getAdapter(ICommandService.class);
		IContextService contextSupport = (IContextService)workbench.getAdapter(IContextService.class);
		
		if (commandSupport != null && contextSupport != null)
		{
			fContext.add(contextSupport.activateContext(ID_ASYNC_TABLE_RENDERING_CONTEXT));
			Command gotoCommand = commandSupport.getCommand(ID_GO_TO_ADDRESS_COMMAND);

			if (fGoToAddressHandler == null)
			{
				fGoToAddressHandler = new AbstractHandler() {
					public Object execute(ExecutionEvent event) throws ExecutionException {
						if (fSashForm.getMaximizedControl() != null)
							fGoToAddressAction.run();
						else
							hideGotoAddressComposite();
						return null;
					}};
			}
			gotoCommand.setHandler(fGoToAddressHandler);
		}
		
	}


	public void deactivated() {
		
		fActivated = false;
    	IWorkbench workbench = PlatformUI.getWorkbench();
		ICommandService commandSupport = (ICommandService)workbench.getAdapter(ICommandService.class);
		IContextService contextSupport = (IContextService)workbench.getAdapter(IContextService.class);
		
		if (commandSupport != null && contextSupport != null)
		{
			// 	remove handler
			Command command = commandSupport.getCommand(ID_GO_TO_ADDRESS_COMMAND);
			command.setHandler(null);

			if (fContext != null)
				contextSupport.deactivateContexts(fContext);
		}
		super.deactivated();
	}
	
	private boolean isActivated()
	{
		return fActivated;
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
